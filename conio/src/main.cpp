#include "core.h"
#include "blunder.h"
#include "io.h"
#include "testable.h"

#include <optional>

//////////////////////////////////////////////////////////////////////////

enum ai_type
{
  ait_player,
  ait_random,
  ait_minimax,
  ait_alphabeta,
  ait_complex,
};

template <bool IsWhite>
void perform_move(chess_board &board, list<chess_move> &moves, const ai_type from_input);

void print_board(const chess_board &board);
char read_char();
lsResult read_start_position_from_file(const char *filename, chess_board &board);
chess_move get_move_from_input(const chess_board &board, list<chess_move> &moves);

//////////////////////////////////////////////////////////////////////////

int32_t main(const int32_t argc, char **pArgv)
{
  sformatState_ResetCulture();
  cpu_info::DetectCpuFeatures();

  if (!cpu_info::avx2Supported || !cpu_info::aesNiSupported)
  {
    print_error_line("CPU '", cpu_info::GetCpuName(), "' is not supported (AVX2, AES/NI required)");
    return EXIT_FAILURE;
  }

  chess_board board = chess_board::get_starting_point();
  ai_type white_player = ait_player;
  ai_type black_player = ait_alphabeta;

  bool runTests = true;

  for (size_t i = 1; i < (size_t)argc; i++)
  {
    if (lsStringEquals("--play-white", pArgv[i]))
      white_player = ait_player;
    else if (lsStringEquals("--play-black", pArgv[i]))
      black_player = ait_player;
    else if (lsStringEquals("--random-white", pArgv[i]))
      white_player = ait_random;
    else if (lsStringEquals("--random-black", pArgv[i]))
      black_player = ait_random;
    else if (lsStringEquals("--minimax-white", pArgv[i]))
      white_player = ait_minimax;
    else if (lsStringEquals("--minimax-black", pArgv[i]))
      black_player = ait_minimax;
    else if (lsStringEquals("--alphabeta-white", pArgv[i]))
      white_player = ait_alphabeta;
    else if (lsStringEquals("--alphabeta-black", pArgv[i]))
      black_player = ait_alphabeta;
    else if (lsStringEquals("--complex-white", pArgv[i]))
      white_player = ait_complex;
    else if (lsStringEquals("--complex-black", pArgv[i]))
      black_player = ait_complex;
    else if (lsStringEquals("--no-tests", pArgv[i]))
      runTests = false;
    else if (LS_FAILED(read_start_position_from_file(pArgv[i], board)))
      lsFail();
  }

  if (runTests)
    run_testables();

  // Set Working Directory.
  do
  {
    wchar_t filePath[MAX_PATH];
    GetModuleFileNameW(nullptr, filePath, sizeof(filePath) / sizeof(wchar_t));

    if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
    {
      print_error_line("Insufficient Buffer for Module File Name Retrieval.");
      break;
    }

    wchar_t *lastSlash = nullptr;

    for (size_t i = 0; i < sizeof(filePath) / sizeof(wchar_t) && filePath[i] != L'\0'; i++)
      if (filePath[i] == L'\\')
        lastSlash = filePath + i;

    if (lastSlash != nullptr)
      *lastSlash = L'\0';

    if (0 == SetCurrentDirectoryW(filePath))
      print_error_line("Failed to set working directory.");

  } while (false);

  print("Blunder ConIO (built " __DATE__ " " __TIME__ ") running on ", cpu_info::GetCpuName(), ".\n");

  list<chess_move> moves;
  print_board(board);

  while (true)
  {
    perform_move<true>(board, moves, white_player);

    if (board.hasWhiteWon)
      break;

    perform_move<false>(board, moves, black_player);

    if (board.hasBlackWon)
      break;
  }

  print(board.hasBlackWon ? "Black" : "White", " has won the game!\n");

  return EXIT_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////

void print_played_move(const chess_move move)
{
  print("Played Move: ", (char)(move.startX + 'a'), move.startY + 1, (char)(move.targetX + 'a'), move.targetY + 1, "\n\n");
}

chess_move get_move_from_input(const chess_board &board, list<chess_move> &moves)
{
  while (true)
  {
    print("Specify Move: (e.g. e2e4) - You are playing as ", board.isWhitesTurn ? "white" : "black", '\n');

    const uint8_t originX = (uint8_t)(read_char() - 'a');
    const uint8_t originY = (uint8_t)(read_char() - '1');

    if (originX >= 8 || originY >= 8)
    {
      print_error_line("Invalid Origin.");
      continue;
    }

    const uint8_t targetX = (uint8_t)(read_char() - 'a');
    const uint8_t targetY = (uint8_t)(read_char() - '1');

    if (targetX >= 8 || targetY >= 8)
    {
      print_error_line("Invalid Target.");
      continue;
    }

    if (LS_FAILED(get_all_valid_moves(board, moves)))
    {
      print_error_line("Failed to retrieve moves. Aborting.");
      exit(EXIT_FAILURE);
    }

    print("\n");

    std::optional<chess_move> chosenMove;
    std::optional<chess_piece_type> chosenPromotion;

    for (const auto &move : moves)
    {
      if (move.startX == originX && move.startY == originY && move.targetX == targetX && move.targetY == targetY)
      {
        if (move.isPromotion)
        {
          if (!chosenPromotion.has_value())
          {
            print("Promote to: ([q]ueen/k[n]ight)\n");
            const char piece = (char)read_char();
            switch (piece)
            {
            case 'q':
              chosenPromotion = cpT_queen;
              break;

            case 'k':
            case 'n':
              chosenPromotion = cpT_knight;
              break;

            default:
              chosenPromotion = cpT_none;
              break;
            }

            print("\n");
          }

          if (move.isPromotedToQueen != (chosenPromotion == cpT_queen))
            continue;
        }

        chosenMove = move;
        break;
      }
    }

    if (!chosenMove.has_value())
    {
      print_error_line("Invalid Move!");
      continue;
    }

    return chosenMove.value();
  }
}

template <bool IsWhite>
void perform_move(chess_board &board, list<chess_move> &moves, const ai_type ai)
{
  switch (ai)
  {
  default:
  case ait_player:
  {
    const chess_move move = get_move_from_input(board, moves);
    board = perform_move(board, move);
    break;
  }

  case ait_random:
  {
    if (LS_FAILED(get_all_valid_moves(board, moves)))
    {
      print_error_line("Failed to retrieve moves. Aborting.");
      exit(EXIT_FAILURE);
    }

    // Perform random move.
    const size_t moveIdx = lsGetRand() % moves.count;
    board = perform_move(board, moves[moveIdx]);
    print_played_move(moves[moveIdx]);

    break;
  }

  case ait_minimax:
  {
    chess_move move;

    if constexpr (IsWhite)
      move = get_minimax_move_white(board);
    else
      move = get_minimax_move_black(board);

    board = perform_move(board, move);
    print_played_move(move);

    break;
  }

  case ait_alphabeta:
  {
    chess_move move;

    if constexpr (IsWhite)
      move = get_alpha_beta_move_white(board);
    else
      move = get_alpha_beta_move_black(board);

    board = perform_move(board, move);
    print_played_move(move);

    break;
  }

  case ait_complex:
  {
    chess_move move;

    if constexpr (IsWhite)
      move = get_complex_move_white(board);
    else
      move = get_complex_move_black(board);

    board = perform_move(board, move);
    print_played_move(move);

    break;
  }
  }

  print_board(board);
}

//////////////////////////////////////////////////////////////////////////

lsResult read_start_position_from_file(const char *filename, chess_board &board)
{
  lsResult result = lsR_Success;

  char *fileContents = nullptr;
  size_t _unused;

  print_log_line("Trying to read starting Position from file: ", filename);

  LS_ERROR_CHECK(lsReadFile(filename, &fileContents, &_unused));

  board = get_board_from_starting_position(fileContents);

epilogue:
  return result;
}

//lsResult parse_fen_book(const char *filename, chess_hash_board *pBoards, const size_t count)
//{
//  lsResult result = lsR_Success;
//
//  print_log_line("Trying to read board from file: ", filename);
//
//  char *fileContents = nullptr;
//  size_t fileSize;
//
//  LS_ERROR_CHECK(lsReadFile(filename, &fileContents, &fileSize));
//
//  lsZeroMemory(&pBoards);
//
//  {
//    size_t addedBoardCount = 0;
//
//    while (addedBoardCount < count)
//    {
//      chess_board b;
//
//      // turn board to hashboard
//      // hash the board
//      // if board does not already exist
//        // save to pBoards
//
//      // find end of line
//    }
//  }
//
//epilogue:
//  return result;
//}
