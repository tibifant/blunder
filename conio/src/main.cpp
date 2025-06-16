#include "core.h"
#include "blunder.h"

#include <conio.h>

#include <optional>

//////////////////////////////////////////////////////////////////////////

void print_board(const chess_board &board);
char read_char();

//////////////////////////////////////////////////////////////////////////

int32_t main(const int32_t argc, char **pArgv)
{
  // Ignore Args for now.
  (void)argc;
  (void)pArgv;

  sformatState_ResetCulture();
  cpu_info::DetectCpuFeatures();

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

  chess_board board = chess_board::get_starting_point();
  small_list<chess_move> moves;

  while (true)
  {
    print("\n");
    print_board(board);
    print("Specify Move: (e.g. e2e4)\n");

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
      return EXIT_FAILURE;
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

    // Perform move.
    board = perform_move(board, chosenMove.value());
    print_board(board);

    if (board.hasWhiteWon)
      break;

    // AI move.
    {
      print("Calculating AI Move... (lol)\n");

      if (LS_FAILED(get_all_valid_moves(board, moves)))
      {
        print_error_line("Failed to retrieve moves. Aborting.");
        return EXIT_FAILURE;
      }

      const size_t moveIdx = lsGetRand() % moves.count;
      board = perform_move(board, moves[moveIdx]);

      if (board.hasBlackWon)
        break;
    }
  }

  print(board.hasBlackWon ? "Black" : "White", " has won the game!\n");

  return EXIT_SUCCESS;
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
