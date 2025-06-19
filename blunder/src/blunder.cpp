#include "blunder.h"

constexpr vec2i8 TopLeftRelative = vec2i8(-1, -1);
constexpr vec2i8 TopRelative = vec2i8(0, -1);
constexpr vec2i8 TopRightRelative = vec2i8(1, -1);
constexpr vec2i8 LeftRelative = vec2i8(-1, 0);
constexpr vec2i8 RightRelative = vec2i8(1, 0);
constexpr vec2i8 BottomLeftRelative = vec2i8(-1, 1);
constexpr vec2i8 BottomRelative = vec2i8(0, 1);
constexpr vec2i8 BottomRightRelative = vec2i8(1, 1);

inline bool is_check_straight(const chess_board &board, const vec2i8 pos, const bool isWhite, const vec2i8 dir)
{
  lsAssert(pos.x >= 0 && pos.x < BoardWidth && pos.y >= 0 && pos.y < BoardWidth);

  for (vec2i8 t = pos; t.x >= 0 && t.x < BoardWidth && t.y >= 0 && t.y < BoardWidth; t += dir)
  {
    const chess_piece p = board[t];

    if (p.piece)
    {
      if (p.isWhite == isWhite)
        break;

      if (p.isWhite != isWhite)
        if (p.piece == cpT_rook || p.piece == cpT_queen)
          return true;
    }
  }

  return false;
}

inline bool is_check_diagonal(const chess_board &board, const vec2i8 pos, const bool isWhite, const vec2i8 dir)
{
  lsAssert(pos.x >= 0 && pos.x < BoardWidth && pos.y >= 0 && pos.y < BoardWidth);

  for (vec2i8 t = pos; t.x >= 0 && t.x < BoardWidth && t.y >= 0 && t.y < BoardWidth; t += dir)
  {
    const chess_piece p = board[dir];

    if (p.piece)
    {
      if (p.isWhite == isWhite)
        break;

      if (p.isWhite != isWhite)
        if (p.piece == cpT_bishop || p.piece == cpT_queen)
          return true;
    }
  }

  return false;
}

bool is_check_for_position(const chess_board &board, const vec2i8 pos, const bool isWhite)
{
  lsAssert(pos.x >= 0 && pos.x < BoardWidth && pos.y >= 0 && pos.y < BoardWidth);

  if (is_check_straight(board, pos, isWhite, vec2i8(0, 1)))
    return true;

  if (is_check_straight(board, pos, isWhite, vec2i8(0, -1)))
    return true;

  if (is_check_straight(board, pos, isWhite, vec2i8(1, 0)))
    return true;

  if (is_check_straight(board, pos, isWhite, vec2i8(-1, 0)))
    return true;

  if (is_check_diagonal(board, pos, isWhite, vec2i8(1, 1)))
    return true;

  if (is_check_diagonal(board, pos, isWhite, vec2i8(-1, -1)))
    return true;

  // pawns
  const vec2i8 pawnPosLeft = isWhite ? pos + vec2i8(-1, 1) : vec2i8(-1, -1);
  const vec2i8 pawnPosRight = isWhite ? pos + vec2i8(1, 1) : vec2i8(1, -1);

  if (pawnPosLeft.y >= 0 && pawnPosLeft.y < BoardWidth) // y is the same for left and right
  {
    if (pawnPosLeft.x >= 0 && board[pawnPosLeft].piece == cpT_pawn && board[pawnPosLeft].isWhite != isWhite)
      return true;

    if (pawnPosRight.x < BoardWidth && board[pawnPosLeft].piece == cpT_pawn && board[pawnPosLeft].isWhite != isWhite)
      return true;
  }

  // knights
  constexpr static vec2i8 TargetDir[] = { vec2i8(-2, -1), vec2i8(-1, -2), vec2i8(1, -2), vec2i8(2, -1), vec2i8(2, 1), vec2i8(1, 2), vec2i8(-1, 2), vec2i8(-2, 1) };

  for (size_t i = 0; i < LS_ARRAYSIZE(TargetDir); i++)
  {
    const vec2i8 target = pos + TargetDir[i];

    if (target.x >= 0 && target.y < BoardWidth)
    {
      const chess_piece p = board[target];

      if (p.piece == cpT_knight && p.isWhite != isWhite)
        return true;
    }
  }

  return false;
}

__forceinline lsResult add_valid_move(const vec2i8 origin, const vec2i8 destination, const chess_board &board, small_list<chess_move> &moves)
{
  if (destination.x >= 0 && destination.x < BoardWidth && destination.y >= 0 && destination.y < BoardWidth && (!board[destination].piece || board[destination].isWhite != board.isWhitesTurn))
    return list_add(&moves, chess_move(origin, destination));
  else
    return lsR_Success;
}

__forceinline lsResult add_valid_move(const chess_move move, const chess_board &board, small_list<chess_move> &moves)
{
  const vec2i8 dest = vec2i8(move.targetX, move.targetY);

  if (move.targetX >= 0 && move.targetX < BoardWidth && move.targetY >= 0 && move.targetY < BoardWidth && (!board[dest].piece || board[dest].isWhite != board.isWhitesTurn))
    return list_add(&moves, move);
  else
    return lsR_Success;
}

inline lsResult add_repeated_moves(const chess_board &board, const vec2i8 startPos, const vec2i8 dir, small_list<chess_move> &moves)
{
  lsResult result = lsR_Success;

  lsAssert(startPos.x >= 0 && startPos.x < BoardWidth && startPos.y >= 0 && startPos.y < BoardWidth);

  for (int8_t i = 0; i < BoardWidth; i++)
  {
    const vec2i8 targetPos = startPos + vec2i8(i) * dir;

    LS_ERROR_CHECK(add_valid_move(startPos, targetPos, board, moves));

    if (board[targetPos].piece)
      break;
  }

epilogue:
  return result;
}

__forceinline lsResult get_pawn_moves_from(const chess_board &board, small_list<chess_move> &moves, const vec2i8 startPos)
{
  lsResult result = lsR_Success;

  const vec2i8 dir = board[startPos].isWhite ? vec2i8(0, 1) : vec2i8(0, -1);
  const vec2i8 targetPos = vec2i8(startPos + dir);
  const vec2i8 doubleStepTargetPos = targetPos + dir;
  const vec2i8 diagonalLeftTargetPos = vec2i8(targetPos.x - 1, targetPos.y);
  const vec2i8 diagonalRightTargetPos = vec2i8(targetPos.x + 1, targetPos.y);

  if (!board[targetPos].piece)
  {
    if (((board.isWhitesTurn && startPos.y == 1) || (!board[startPos].isWhite && startPos.y == 6)) && !board[doubleStepTargetPos].piece)
      LS_ERROR_CHECK(add_valid_move(startPos, doubleStepTargetPos, board, moves));

    if (targetPos.y == BoardWidth - 1 || targetPos.y == 0)
    {
      chess_move move = chess_move(startPos, targetPos);
      move.isPromotion = true;

      move.isPromotedToQueen = true;
      LS_ERROR_CHECK(add_valid_move(move, board, moves));

      move.isPromotedToQueen = false;
      LS_ERROR_CHECK(add_valid_move(move, board, moves));
    }
    else
    {
      LS_ERROR_CHECK(add_valid_move(startPos, targetPos, board, moves));
    }
  }

  if (diagonalLeftTargetPos.x >= 0 && diagonalLeftTargetPos.y >= 0 && diagonalLeftTargetPos.y < BoardWidth)
  {
    const chess_piece enemyPiece = board[diagonalLeftTargetPos];

    if (enemyPiece.piece && (enemyPiece.isWhite != board.isWhitesTurn))
      LS_ERROR_CHECK(add_valid_move(startPos, diagonalLeftTargetPos, board, moves));
  }

  if (diagonalRightTargetPos.x < BoardWidth && diagonalRightTargetPos.y >= 0 && diagonalRightTargetPos.y < BoardWidth)
  {
    const chess_piece enemyPiece = board[diagonalRightTargetPos];

    if (enemyPiece.piece && (enemyPiece.isWhite != board.isWhitesTurn))
      LS_ERROR_CHECK(add_valid_move(startPos, diagonalRightTargetPos, board, moves));
  }

  // en passant
  if ((board.isWhitesTurn && startPos.y == 4) || (!board[startPos].isWhite && startPos.y == 3))
  {
    const vec2i8 enemyPosLeft = vec2i8(startPos.x - 1, startPos.y);
    const vec2i8 enemyPosRight = vec2i8(startPos.x + 1, startPos.y);

    if (enemyPosLeft.x >= 0)
    {
      const chess_piece enemyPiece = board[enemyPosLeft];

      if (enemyPiece.piece && enemyPiece.lastWasDoubleStep && (enemyPiece.isWhite != board.isWhitesTurn))
        LS_ERROR_CHECK(add_valid_move(startPos, vec2i8(targetPos.x - 1, targetPos.y), board, moves));
    }

    if (enemyPosRight.x < BoardWidth)
    {
      const chess_piece enemyPiece = board[enemyPosRight];

      if (enemyPiece.piece && enemyPiece.lastWasDoubleStep && (enemyPiece.isWhite != board.isWhitesTurn))
        LS_ERROR_CHECK(add_valid_move(startPos, vec2i8(targetPos.x + 1, targetPos.y), board, moves));
    }
  }

epilogue:
  return result;
}

__forceinline lsResult add_castle_moves_from(const chess_board &board, small_list<chess_move> &moves, const vec2i8 kingStartPos)
{
  lsResult result = lsR_Success;

  lsAssert(kingStartPos.x >= 0 && kingStartPos.y < BoardWidth && board[kingStartPos].piece == cpT_king);
  const chess_piece king = board[kingStartPos];

  if (king.isWhite == board.isWhitesTurn && !king.hasMoved)
  {
    const vec2i8 rookPosRight = king.isWhite ? vec2i8(0, 0) : vec2i8(0, 7);
    const vec2i8 rookPosLeft = king.isWhite ? vec2i8(7, 0) : vec2i8(7, 7);

    if (!board[rookPosLeft].hasMoved)
    {
      bool isFree = true;
      bool isCheck = false;

      for (int8_t x = kingStartPos.x - 1; x > 0; x--)
      {
        const vec2i8 pos = vec2i8(x, kingStartPos.y);

        if (board[pos].piece)
        {
          isFree = false;
          break;
        }

        if (is_check_for_position(board, pos, king.isWhite))
        {
          isCheck = true;
          break;
        }
      }

      if (isFree && !isCheck)
        LS_ERROR_CHECK(list_add(&moves, chess_move(kingStartPos, rookPosLeft + vec2i8(1, 0)))); // all checks from `add_valid_move` have already been checked
    }

    if (!board[rookPosRight].hasMoved)
    {
      bool isFree = true;
      bool isCheck = false;

      for (int8_t x = kingStartPos.x + 1; x < BoardWidth - 1; x++)
      {
        const vec2i8 pos = vec2i8(x, kingStartPos.y);

        if (board[pos].piece)
        {
          isFree = false;
          break;
        }

        if (is_check_for_position(board, pos, king.isWhite))
        {
          isCheck = true;
          break;
        }
      }

      if (isFree && !isCheck)
        LS_ERROR_CHECK(list_add(&moves, chess_move(kingStartPos, rookPosRight + vec2i8(-1, 0)))); // all checks from `add_valid_move` have already been checked
    }
  }

epilogue:
  return result;
}

template <chess_piece_type piece>
lsResult get_all_valid_piece_moves(const chess_board &board, small_list<chess_move> &moves)
{
  lsResult result = lsR_Success;

  for (int8_t y = 0; y < BoardWidth; y++)
  {
    for (int8_t x = 0; x < BoardWidth; x++)
    {
      const vec2i8 startPos = vec2i8(x, y);

      if (board[startPos].piece == piece && board[startPos].isWhite == board.isWhitesTurn)
      {
        if constexpr (piece == cpT_pawn)
        {
          LS_ERROR_CHECK(get_pawn_moves_from(board, moves, startPos));
        }
        else if constexpr (piece == cpT_knight)
        {
          constexpr static vec2i8 TargetDir[] = { vec2i8(-2, -1), vec2i8(-1, -2), vec2i8(1, -2), vec2i8(2, -1), vec2i8(2, 1), vec2i8(1, 2), vec2i8(-1, 2), vec2i8(-2, 1) };

          for (size_t i = 0; i < LS_ARRAYSIZE(TargetDir); i++)
            LS_ERROR_CHECK(add_valid_move(startPos, vec2i8(startPos + TargetDir[i]), board, moves));
        }
        else if constexpr (piece == cpT_bishop || piece == cpT_rook || piece == cpT_queen)
        {
          if constexpr (piece != cpT_rook)
          {
            LS_ERROR_CHECK(add_repeated_moves(board, startPos, TopLeftRelative, moves));
            LS_ERROR_CHECK(add_repeated_moves(board, startPos, TopRightRelative, moves));
            LS_ERROR_CHECK(add_repeated_moves(board, startPos, BottomLeftRelative, moves));
            LS_ERROR_CHECK(add_repeated_moves(board, startPos, BottomRightRelative, moves));
          }

          if constexpr (piece != cpT_bishop)
          {
            LS_ERROR_CHECK(add_repeated_moves(board, startPos, LeftRelative, moves));
            LS_ERROR_CHECK(add_repeated_moves(board, startPos, TopRelative, moves));
            LS_ERROR_CHECK(add_repeated_moves(board, startPos, RightRelative, moves));
            LS_ERROR_CHECK(add_repeated_moves(board, startPos, BottomRelative, moves));
          }
        }
        else if constexpr (piece == cpT_king)
        {
          constexpr vec2i8 TargetDir[] = { TopLeftRelative, TopRelative, TopRightRelative, LeftRelative, RightRelative, BottomLeftRelative, BottomRelative, BottomRightRelative };

          for (size_t i = 0; i < LS_ARRAYSIZE(TargetDir); i++)
            LS_ERROR_CHECK(add_valid_move(startPos, vec2i8(startPos + TargetDir[i]), board, moves));

          LS_ERROR_CHECK(add_castle_moves_from(board, moves, startPos));
        }
        else
        {
          static_assert(piece == cpT_none);
          lsFail();
        }
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

  LS_ERROR_CHECK(get_all_valid_piece_moves<cpT_king>(board, moves));
  LS_ERROR_CHECK(get_all_valid_piece_moves<cpT_queen>(board, moves));
  LS_ERROR_CHECK(get_all_valid_piece_moves<cpT_rook>(board, moves));
  LS_ERROR_CHECK(get_all_valid_piece_moves<cpT_bishop>(board, moves));
  LS_ERROR_CHECK(get_all_valid_piece_moves<cpT_knight>(board, moves));
  LS_ERROR_CHECK(get_all_valid_piece_moves<cpT_pawn>(board, moves));

epilogue:
  return result;
}

chess_board perform_move(const chess_board &board, const chess_move move)
{
  chess_board ret = board;
  ret.isWhitesTurn = (uint8_t)!board.isWhitesTurn;

  for (size_t i = 0; i < LS_ARRAYSIZE(ret.board); i++)
    ret.board[i].lastWasDoubleStep = false;

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

    if (move.startX != move.targetX)
    {
      if (target.piece)
      {
        lsAssert(target.isWhite == ret.isWhitesTurn);
      }
      else // en passant
      {
        const vec2i8 enemyPos = vec2i8(move.targetX, move.startY);
        lsAssert(board[enemyPos].piece && board[enemyPos].lastWasDoubleStep && board[enemyPos].piece == cpT_pawn && (board[enemyPos].isWhite == ret.isWhitesTurn));
        ret[enemyPos].piece = cpT_none;
      }
    }
    else if (move.isPromotion)
    {
      lsAssert((board.isWhitesTurn && move.targetY == BoardWidth - 1) || (!board.isWhitesTurn && move.targetY == 0));

      if (move.isPromotedToQueen)
        origin.piece = cpT_queen;
      else
        origin.piece = cpT_knight;
    }
  }
  else if (origin.piece == cpT_king)
  {
    // castlen
    if (lsAbs(move.startX - move.targetX) > 1)
    {
      lsAssert((move.targetY == 0 || move.targetY == BoardWidth - 1) && (move.targetX == 1 || move.targetX == BoardWidth - 2));

      const vec2i8 rookPosOrigin = move.targetX == BoardWidth - 2 ? vec2i8(move.targetX + 1, move.targetY) : vec2i8(move.targetX - 1, move.targetY);
      const vec2i8 rookPosTarget = move.targetX == BoardWidth - 2 ? vec2i8(move.targetX - 1, move.targetY) : vec2i8(move.targetX + 1, move.targetY);
      lsAssert(board[rookPosOrigin].piece == cpT_rook);

      chess_piece &rookOrigin = ret[rookPosOrigin];
      chess_piece &rookTarget = ret[rookPosTarget];

      rookOrigin.hasMoved = true;
      rookTarget = std::move(rookOrigin);
      ret[rookPosOrigin].piece = cpT_none;
    }
  }

  origin.hasMoved = true;
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
