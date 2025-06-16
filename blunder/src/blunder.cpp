#include "blunder.h"

constexpr vec2i8 TopLeftRelative = vec2i8(-1, -1);
constexpr vec2i8 TopRelative = vec2i8(0, -1);
constexpr vec2i8 TopRightRelative = vec2i8(1, -1);
constexpr vec2i8 LeftRelative = vec2i8(-1, 0);
constexpr vec2i8 RightRelative = vec2i8(1, 0);
constexpr vec2i8 BottomLeftRelative = vec2i8(-1, 1);
constexpr vec2i8 BottomRelative = vec2i8(0, 1);
constexpr vec2i8 BottomRightRelative = vec2i8(1, 1);

lsResult add_repeated_moves(const chess_board &board, const vec2i8 startPos, const vec2i8 dir, small_list<chess_move> &moves)
{
  lsResult result = lsR_Success;

  lsAssert(startPos.x >= 0 && startPos.x < BoardWidth && startPos.y >= 0 && startPos.y < BoardWidth);

  for (int8_t i = 0; i < 8; i++)
  {
    const vec2i8 targetPos = startPos + vec2i8(i) * dir;

    if (targetPos.x >= 0 && targetPos.x < BoardWidth && targetPos.y >= 0 && targetPos.y < BoardWidth && (!board[targetPos].piece || board[targetPos].isWhite != board.isWhitesTurn))
      LS_ERROR_CHECK(list_add(&moves, chess_move(startPos, targetPos)));
  }

epilogue:
  return result;
}

lsResult get_all_valid_king_moves(const chess_board &board, small_list<chess_move> &moves)
{
  lsResult result = lsR_Success;

  for (int8_t y = 0; y < BoardWidth; y++)
  {
    for (int8_t x = 0; x < BoardWidth; x++)
    {
      const vec2i8 startPos = vec2i8(x, y);

      if (board[startPos].piece == cpT_king && board[startPos].isWhite == board.isWhitesTurn)
      {
        constexpr vec2i8 KingPossibleTargetPos[] = { TopLeftRelative, TopRelative, TopRightRelative, LeftRelative, RightRelative, BottomLeftRelative, BottomRelative, BottomRightRelative };

        for (size_t j = 0; j < LS_ARRAYSIZE(KingPossibleTargetPos); j++)
        {
          const vec2i8 targetPos = startPos + KingPossibleTargetPos[j];

          if (targetPos.x >= 0 && targetPos.x < BoardWidth && targetPos.y >= 0 && targetPos.y < BoardWidth && (!board[targetPos].piece || board[targetPos].isWhite != board.isWhitesTurn))
            LS_ERROR_CHECK(list_add(&moves, chess_move(startPos, targetPos)));
        }
      }
    }
  }

epilogue:
  return result;
}

lsResult get_all_valid_queen_moves(const chess_board &board, small_list<chess_move> &moves)
{
  lsResult result = lsR_Success;

  for (int8_t y = 0; y < BoardWidth; y++)
  {
    for (int8_t x = 0; x < BoardWidth; x++)
    {
      const vec2i8 startPos = vec2i8(x, y);

      if (board[startPos].piece == cpT_queen && board[startPos].isWhite == board.isWhitesTurn)
      {
        LS_ERROR_CHECK(add_repeated_moves(board, startPos, TopLeftRelative, moves));
        LS_ERROR_CHECK(add_repeated_moves(board, startPos, TopRelative, moves));
        LS_ERROR_CHECK(add_repeated_moves(board, startPos, TopRightRelative, moves));
        LS_ERROR_CHECK(add_repeated_moves(board, startPos, LeftRelative, moves));
        LS_ERROR_CHECK(add_repeated_moves(board, startPos, RightRelative, moves));
        LS_ERROR_CHECK(add_repeated_moves(board, startPos, BottomLeftRelative, moves));
        LS_ERROR_CHECK(add_repeated_moves(board, startPos, BottomRelative, moves));
        LS_ERROR_CHECK(add_repeated_moves(board, startPos, BottomRightRelative, moves));
      }
    }
  }

epilogue:
  return result;
}

lsResult get_all_valid_rook_moves(const chess_board &board, small_list<chess_move> &moves)
{
  lsResult result = lsR_Success;

  for (int8_t y = 0; y < BoardWidth; y++)
  {
    for (int8_t x = 0; x < BoardWidth; x++)
    {
      const vec2i8 startPos = vec2i8(x, y);

      if (board[startPos].piece == cpT_rook && board[startPos].isWhite == board.isWhitesTurn)
      {
        LS_ERROR_CHECK(add_repeated_moves(board, startPos, LeftRelative, moves));
        LS_ERROR_CHECK(add_repeated_moves(board, startPos, TopRelative, moves));
        LS_ERROR_CHECK(add_repeated_moves(board, startPos, RightRelative, moves));
        LS_ERROR_CHECK(add_repeated_moves(board, startPos, BottomRelative, moves));
      }
    }
  }

epilogue:
  return result;
}

lsResult get_all_valid_bishop_moves(const chess_board &board, small_list<chess_move> &moves)
{
  lsResult result = lsR_Success;

  for (int8_t y = 0; y < BoardWidth; y++)
  {
    for (int8_t x = 0; x < BoardWidth; x++)
    {
      const vec2i8 startPos = vec2i8(x, y);

      if (board[startPos].piece == cpT_bishop && board[startPos].isWhite == board.isWhitesTurn)
      {
        LS_ERROR_CHECK(add_repeated_moves(board, startPos, TopLeftRelative, moves));
        LS_ERROR_CHECK(add_repeated_moves(board, startPos, TopRightRelative, moves));
        LS_ERROR_CHECK(add_repeated_moves(board, startPos, BottomLeftRelative, moves));
        LS_ERROR_CHECK(add_repeated_moves(board, startPos, BottomRightRelative, moves));
      }
    }
  }

epilogue:
  return result;
}

lsResult get_all_valid_knight_moves(const chess_board &board, small_list<chess_move> &moves)
{
  lsResult result = lsR_Success;

  for (int8_t y = 0; y < BoardWidth; y++)
  {
    for (int8_t x = 0; x < BoardWidth; x++)
    {
      const vec2i8 startPos = vec2i8(x, y);

      if (board[startPos].piece == cpT_knight && board[startPos].isWhite == board.isWhitesTurn)
      {
        constexpr vec2i8 TargetDir[] = { vec2i8(-2, -1), vec2i8(-1, -2), vec2i8(1, -2), vec2i8(2, -1), vec2i8(2, 1), vec2i8(1, 2), vec2i8(1, 2), vec2i8(-1, 2), vec2i8(-2, 1) };

        for (size_t i = 0; i < LS_ARRAYSIZE(TargetDir); i++)
        {
          const vec2i8 targetPos = startPos + TargetDir[i];

          if (targetPos.x >= 0 && targetPos.x < BoardWidth && targetPos.y >= 0 && targetPos.y < BoardWidth && (!board[targetPos].piece || board[targetPos].isWhite != board.isWhitesTurn))
            LS_ERROR_CHECK(list_add(&moves, chess_move(startPos, targetPos)));
        }
      }
    }
  }

epilogue:
  return result;

}

lsResult get_all_valid_pawn_moves(const chess_board &board, small_list<chess_move> &moves)
{
  lsResult result = lsR_Success;

  for (int8_t y = 0; y < BoardWidth; y++)
  {
    for (int8_t x = 0; x < BoardWidth; x++)
    {
      const vec2i8 startPos = vec2i8(x, y);

      if (board[startPos].piece == cpT_pawn && board[startPos].isWhite == board.isWhitesTurn)
      {
        const vec2i8 dir = board[startPos].isWhite ? vec2i8(0, 1) : vec2i8(0, -1);

        if ((board.isWhitesTurn && startPos.y == 1) || (!board[startPos].isWhite && startPos.y == 6))
        {
          const vec2i8 targetPos = startPos + vec2i8(2) * dir;

          if (targetPos.x >= 0 && targetPos.x < BoardWidth && targetPos.y >= 0 && targetPos.y < BoardWidth && (!board[targetPos].piece || board[targetPos].isWhite != board.isWhitesTurn))
          {
            LS_ERROR_CHECK(list_add(&moves, chess_move(startPos, targetPos)));
          }
        }

        const vec2i8 targetPos = startPos + dir;

        if (targetPos.x >= 0 && targetPos.x < BoardWidth && targetPos.y >= 0 && targetPos.y < BoardWidth && (!board[targetPos].piece || board[targetPos].isWhite != board.isWhitesTurn))
          LS_ERROR_CHECK(list_add(&moves, chess_move(startPos, targetPos)));
      }
    }
  }

epilogue:
  return result;
}

lsResult get_all_valid_moves(const chess_board &board, small_list<chess_move> &moves)
{
  lsResult result = lsR_Success;

  list_clear(&moves);

  LS_ERROR_CHECK(get_all_valid_king_moves(board, moves));
  LS_ERROR_CHECK(get_all_valid_queen_moves(board, moves));
  LS_ERROR_CHECK(get_all_valid_rook_moves(board, moves));
  LS_ERROR_CHECK(get_all_valid_bishop_moves(board, moves));
  LS_ERROR_CHECK(get_all_valid_knight_moves(board, moves));
  LS_ERROR_CHECK(get_all_valid_pawn_moves(board, moves));

epilogue:
  return result;
}

chess_board perform_move(const chess_board &board, const chess_move move)
{
  chess_board ret = board;
  ret.isWhitesTurn = (uint8_t)!board.isWhitesTurn;

  chess_piece &origin = ret[vec2i8(move.startX, move.startY)];
  chess_piece &target = ret[vec2i8(move.targetX, move.targetY)];
  lsAssert(origin.isWhite == board.isWhitesTurn);

  if (target.piece == cpT_king)
  {
    size_t kingCount = 0;

    for (size_t i = 0; i < LS_ARRAYSIZE(ret.board); i++)
      kingCount += (size_t)(ret.board[i] == target);

    if (kingCount == 1)
    {
      if (target.isWhite)
        ret.hasBlackWon = true;
      else
        ret.hasWhiteWon = true;
    }
  }

  if (origin.piece == cpT_pawn)
  {
    if (((move.startY == 1 && origin.isWhite) || (move.startY == 6 && !origin.isWhite)) && lsAbs(move.startY - move.targetY) == 2)
      origin.lastWasDoubleStep = true;
  }

  target = std::move(origin);

  return ret;
}

inline void place_symmetric_last_row(chess_board &board, const chess_piece_type piece, const int8_t x)
{
  board[vec2i8(x, 0)] = chess_piece(piece, true);
  board[vec2i8(x, BoardWidth - 1)] = chess_piece(piece, false);
}

chess_board chess_board::get_starting_point()
{
  chess_board board;
  board.isWhitesTurn = true;

  const chess_piece_type lastRow[] = { cpT_rook, cpT_knight, cpT_bishop, cpT_queen, cpT_king, cpT_bishop, cpT_knight, cpT_rook };
  static_assert(LS_ARRAYSIZE(lastRow) == BoardWidth);

  for (int8_t i = 0; i < BoardWidth; i++)
  {
    board[vec2i8(i, 1)] = chess_piece(cpT_pawn, true);
    board[vec2i8(i, BoardWidth - 2)] = chess_piece(cpT_pawn, false);

    place_symmetric_last_row(board, lastRow[i], i);
  }

  return board;
}
