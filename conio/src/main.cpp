#include "core.h"
#include "blunder.h"
#include "io.h"

#include <conio.h>

#include <optional>

//////////////////////////////////////////////////////////////////////////

void perform_move(chess_board &board, small_list<chess_move> &moves, const bool from_input);
void print_board(const chess_board &board);
char read_char();
lsResult read_start_position_from_file(const char *filename, chess_board &board);
chess_move get_move_from_input(const chess_board &board, small_list<chess_move> &moves);

//////////////////////////////////////////////////////////////////////////

int32_t main(const int32_t argc, char **pArgv)
{
  sformatState_ResetCulture();
  cpu_info::DetectCpuFeatures();

  chess_board board = chess_board::get_starting_point();
  bool white_from_input = true;
  bool black_from_input = false;

  for (size_t i = 1; i < (size_t)argc; i++)
  {
    if (lsStringEquals("--play-white", pArgv[i]))
      white_from_input = true;
    else if (lsStringEquals("--play-black", pArgv[i]))
      black_from_input = true;
    else if (lsStringEquals("--ai-white", pArgv[i]))
      white_from_input = false;
    else if (lsStringEquals("--ai-black", pArgv[i]))
      black_from_input = false;
    else if (LS_FAILED(read_start_position_from_file(pArgv[i], board)))
      lsFail();
  }

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

  small_list<chess_move> moves;
  print_board(board);

  while (true)
  {
    perform_move(board, moves, white_from_input);

    if (board.hasWhiteWon)
      break;

    perform_move(board, moves, black_from_input);

    if (board.hasBlackWon)
      break;
  }

  print(board.hasBlackWon ? "Black" : "White", " has won the game!\n");

  return EXIT_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////

chess_move get_move_from_input(const chess_board &board, small_list<chess_move> &moves)
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

void perform_move(chess_board &board, small_list<chess_move> &moves, const bool from_input)
{
  if (from_input)
  {
    const chess_move move = get_move_from_input(board, moves);

    // Perform move.
    board = perform_move(board, move);
  }
  else // AI move.
  {
    print("Calculating AI Move... (lol)\n");

    if (LS_FAILED(get_all_valid_moves(board, moves)))
    {
      print_error_line("Failed to retrieve moves. Aborting.");
      exit(EXIT_FAILURE);
    }

    // Perform random move.
    const size_t moveIdx = lsGetRand() % moves.count;
    board = perform_move(board, moves[moveIdx]);
  }

  print_board(board);
}

//////////////////////////////////////////////////////////////////////////

void print_x_coords()
{
  print("   ");

  for (int8_t x = 0; x < 8; x++)
    print(' ', (char)('a' + x), ' ');

  print('\n');
}

void print_board(const chess_board &board)
{
  char pieces[] = " KQRBNP";
  static_assert(LS_ARRAYSIZE(pieces) == _chess_piece_type_count + 1 /* \0 */);

  print_x_coords();

  for (int8_t y = 7; y >= 0; y--)
  {
    print(' ', (char)('1' + y), ' ');

    for (int8_t x = 0; x < 8; x++)
    {
      const chess_piece piece = board[vec2i8(x, y)];
      lsSetConsoleColor(piece.isWhite ? lsCC_White : lsCC_Black, ((x ^ y) & 1) ? lsCC_BrightCyan : lsCC_DarkBlue);
      lsAssert(piece.piece < _chess_piece_type_count);
      print(' ', pieces[piece.piece], ' ');
    }

    lsResetConsoleColor();

    print(' ', (char)('1' + y), '\n');
  }

  print_x_coords();
  print("\n");
}

char read_char()
{
  const char c = (char)_getch();
  print(c);
  return c;
}

//////////////////////////////////////////////////////////////////////////

bool is_upper_case(const char c)
{
  return 'A' <= c && c <= 'Z';
}

lsResult read_start_position_from_file(const char *filename, chess_board &board)
{
  lsResult result = lsR_Success;

  char *fileContents = nullptr;
  size_t fileSize;

  print_log_line("Trying to read starting Position from file: ", filename);

  LS_ERROR_CHECK(lsReadFile(filename, &fileContents, &fileSize));

  {
    vec2i8 currentPos = vec2i8(0, 7);

    for (size_t i = 0; i < fileSize; i++)
    {
      chess_piece_type piece = cpT_none;
      bool isWhite = false;;

      switch (fileContents[i])
      {
      case '.':
      case ' ':
        piece = cpT_none;
        break;

      case 'K':
      case 'k':
        piece = cpT_king;
        isWhite = is_upper_case(fileContents[i]);
        break;

      case 'Q':
      case 'q':
        piece = cpT_queen;
        isWhite = is_upper_case(fileContents[i]);
        break;

      case 'N':
      case 'n':
        piece = cpT_knight;
        isWhite = is_upper_case(fileContents[i]);
        break;

      case 'B':
      case 'b':
        piece = cpT_bishop;
        isWhite = is_upper_case(fileContents[i]);
        break;

      case 'R':
      case 'r':
        piece = cpT_rook;
        isWhite = is_upper_case(fileContents[i]);
        break;

      case 'P':
      case 'p':
        piece = cpT_pawn;
        isWhite = is_upper_case(fileContents[i]);
        break;

      case '\r':
        continue;

      case '\n':
        currentPos.x = 0;
        currentPos.y--;
        continue;

      default:
        print_error_line("Unexpected Token in file: ", fileContents[i]);
        lsFail();
      }

      lsAssert(currentPos.x < BoardWidth && currentPos.y < BoardWidth);

      board[lsMin(currentPos, vec2i8(BoardWidth - 1, BoardWidth - 1))] = chess_piece(piece, isWhite);
      currentPos.x++;
    }
  }

epilogue:
  return result;
}
