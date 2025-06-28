#include "blunder.h"
#include "testable.h"

#include <conio.h>

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
    const chess_piece p = board[t];

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

    if (target.x >= 0 && target.x < BoardWidth && target.y >= 0 && target.y < BoardWidth)
    {
      const chess_piece p = board[target];

      if (p.piece == cpT_knight && p.isWhite != isWhite)
        return true;
    }
  }

  return false;
}

//////////////////////////////////////////////////////////////////////////

__forceinline lsResult add_valid_move(const vec2i8 origin, const vec2i8 destination, const chess_board &board, small_list<chess_move> &moves, [[maybe_unused]] const chess_move_type type)
{
  if (destination.x >= 0 && destination.x < BoardWidth && destination.y >= 0 && destination.y < BoardWidth && (!board[destination].piece || board[destination].isWhite != board.isWhitesTurn))
    return list_add(&moves, chess_move(origin, destination, type));
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

inline lsResult add_repeated_moves(const chess_board &board, const vec2i8 startPos, const vec2i8 dir, small_list<chess_move> &moves, [[maybe_unused]] const chess_move_type type)
{
  lsResult result = lsR_Success;

  lsAssert(startPos.x >= 0 && startPos.x < BoardWidth && startPos.y >= 0 && startPos.y < BoardWidth);

  for (int8_t i = 1; i < BoardWidth; i++)
  {
    const vec2i8 targetPos = startPos + vec2i8(i) * dir;

    if (targetPos.x >= 0 && targetPos.x < BoardWidth && targetPos.y >= 0 && targetPos.y < BoardWidth)
    {
      LS_ERROR_CHECK(add_valid_move(startPos, targetPos, board, moves, type));

      if (board[targetPos].piece)
        break;
    }
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
      LS_ERROR_CHECK(add_valid_move(startPos, doubleStepTargetPos, board, moves, cmt_pawn_double_step));

    if (targetPos.y == BoardWidth - 1 || targetPos.y == 0)
    {
      chess_move move = chess_move(startPos, targetPos, cmt_pawn_promotion);
      move.isPromotion = true;

      move.isPromotedToQueen = true;
      LS_ERROR_CHECK(add_valid_move(move, board, moves));

      move.isPromotedToQueen = false;
      LS_ERROR_CHECK(add_valid_move(move, board, moves));
    }
    else
    {
      LS_ERROR_CHECK(add_valid_move(startPos, targetPos, board, moves, cmt_pawn));
    }
  }

  if (diagonalLeftTargetPos.x >= 0 && diagonalLeftTargetPos.y >= 0 && diagonalLeftTargetPos.y < BoardWidth)
  {
    const chess_piece enemyPiece = board[diagonalLeftTargetPos];

    if (enemyPiece.piece && (enemyPiece.isWhite != board.isWhitesTurn))
      LS_ERROR_CHECK(add_valid_move(startPos, diagonalLeftTargetPos, board, moves, cmt_pawn_capture));
  }

  if (diagonalRightTargetPos.x < BoardWidth && diagonalRightTargetPos.y >= 0 && diagonalRightTargetPos.y < BoardWidth)
  {
    const chess_piece enemyPiece = board[diagonalRightTargetPos];

    if (enemyPiece.piece && (enemyPiece.isWhite != board.isWhitesTurn))
      LS_ERROR_CHECK(add_valid_move(startPos, diagonalRightTargetPos, board, moves, cmt_pawn_capture));
  }

  // en passant
  if ((board.isWhitesTurn && startPos.y == 4) || (!board[startPos].isWhite && startPos.y == 3))
  {
    const vec2i8 enemyPosLeft = vec2i8(startPos.x - 1, startPos.y);
    const vec2i8 enemyPosRight = vec2i8(startPos.x + 1, startPos.y);

    if (enemyPosLeft.x >= 0)
    {
      const chess_piece enemyPiece = board[enemyPosLeft];
      const vec2i8 targetPosLeft = vec2i8(targetPos.x - 1, targetPos.y);

      if (enemyPiece.piece == cpT_pawn && enemyPiece.lastWasDoubleStep && (enemyPiece.isWhite != board.isWhitesTurn))
        LS_ERROR_CHECK(add_valid_move(startPos, targetPosLeft, board, moves, cmt_pawn_en_passant));
    }

    if (enemyPosRight.x < BoardWidth)
    {
      const chess_piece enemyPiece = board[enemyPosRight];
      const vec2i8 targetPosRight = vec2i8(targetPos.x + 1, targetPos.y);

      if (enemyPiece.piece == cpT_pawn && enemyPiece.lastWasDoubleStep && (enemyPiece.isWhite != board.isWhitesTurn))
        LS_ERROR_CHECK(add_valid_move(startPos, targetPosRight, board, moves, cmt_pawn_en_passant));
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
    const vec2i8 rookPosRight = king.isWhite ? vec2i8(BoardWidth - 1, 0) : vec2i8(BoardWidth - 1, BoardWidth - 1);
    const vec2i8 rookPosLeft = king.isWhite ? vec2i8(0, 0) : vec2i8(0, BoardWidth - 1);

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
        LS_ERROR_CHECK(list_add(&moves, chess_move(kingStartPos, rookPosLeft + vec2i8(1, 0), cmt_king_castle))); // all checks from `add_valid_move` have already been checked
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
        LS_ERROR_CHECK(list_add(&moves, chess_move(kingStartPos, rookPosRight + vec2i8(-1, 0), cmt_king_castle))); // all checks from `add_valid_move` have already been checked
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
            LS_ERROR_CHECK(add_valid_move(startPos, vec2i8(startPos + TargetDir[i]), board, moves, cmt_knight));
        }
        else if constexpr (piece == cpT_bishop || piece == cpT_rook || piece == cpT_queen)
        {
          if constexpr (piece != cpT_rook)
          {
            constexpr chess_move_type moveType = piece == cpT_bishop ? cmt_bishop : cmt_queen_diagonal;
            LS_ERROR_CHECK(add_repeated_moves(board, startPos, TopLeftRelative, moves, moveType));
            LS_ERROR_CHECK(add_repeated_moves(board, startPos, TopRightRelative, moves, moveType));
            LS_ERROR_CHECK(add_repeated_moves(board, startPos, BottomLeftRelative, moves, moveType));
            LS_ERROR_CHECK(add_repeated_moves(board, startPos, BottomRightRelative, moves, moveType));
          }

          if constexpr (piece != cpT_bishop)
          {
            constexpr chess_move_type moveType = piece == cpT_rook ? cmt_rook : cmt_queen_straight;
            LS_ERROR_CHECK(add_repeated_moves(board, startPos, LeftRelative, moves, moveType));
            LS_ERROR_CHECK(add_repeated_moves(board, startPos, TopRelative, moves, moveType));
            LS_ERROR_CHECK(add_repeated_moves(board, startPos, RightRelative, moves, moveType));
            LS_ERROR_CHECK(add_repeated_moves(board, startPos, BottomRelative, moves, moveType));
          }
        }
        else if constexpr (piece == cpT_king)
        {
          constexpr vec2i8 TargetDir[] = { TopLeftRelative, TopRelative, TopRightRelative, LeftRelative, RightRelative, BottomLeftRelative, BottomRelative, BottomRightRelative };

          for (size_t i = 0; i < LS_ARRAYSIZE(TargetDir); i++)
            LS_ERROR_CHECK(add_valid_move(startPos, vec2i8(startPos + TargetDir[i]), board, moves, cmt_king));

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

chess_hash_board chess_hash_board_create(const chess_board &board)
{
  chess_hash_board ret;
  ret.isWhitesTurn = board.isWhitesTurn;

  for (size_t i = 0; i < 8 * 4; i++)
  {
    const chess_piece pa = board.board[i * 2];
    const chess_piece pb = board.board[i * 2 + 1];
    const uint8_t a = ((pa.piece & 7) | (pa.isWhite << 3));
    const uint8_t b = (((pb.piece & 7) << 4) | (pb.isWhite << 7));
    ret.nibbleMap[i] = a | b;
  }

  return ret;
}

uint64_t lsHash(const chess_hash_board &board)
{
  __m256i v = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(board.nibbleMap));
  v = _mm256_aesdec_epi128(v, v);
  uint64_t ret = _mm256_extract_epi64(v, 0);
  ret ^= board.isWhitesTurn;
  return ret;
}

lsResult get_all_valid_moves(const chess_board &board, small_list<chess_move> &moves)
{
  lsResult result = lsR_Success;

  list_clear(&moves);

  LS_ERROR_CHECK(get_all_valid_piece_moves<cpT_pawn>(board, moves));
  LS_ERROR_CHECK(get_all_valid_piece_moves<cpT_king>(board, moves));
  LS_ERROR_CHECK(get_all_valid_piece_moves<cpT_queen>(board, moves));
  LS_ERROR_CHECK(get_all_valid_piece_moves<cpT_rook>(board, moves));
  LS_ERROR_CHECK(get_all_valid_piece_moves<cpT_bishop>(board, moves));
  LS_ERROR_CHECK(get_all_valid_piece_moves<cpT_knight>(board, moves));

epilogue:
  return result;
}

//////////////////////////////////////////////////////////////////////////

__forceinline void assert_move_type(const chess_move move, const chess_move_type type, const chess_board &board)
{
#ifdef _DEBUG
  //lsAssert(move.moveType == type);
  if (move.moveType != type)
  {
    print_board(board);
    lsAssert(false);
  }
#else
  (void)board;
  (void)move;
  (void)type;
#endif
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
    {
      assert_move_type(move, cmt_pawn_double_step, board);
      origin.lastWasDoubleStep = true;
    }
    else if (move.startX != move.targetX)
    {
      if (target.piece)
      {
        assert_move_type(move, cmt_pawn_capture, board);
        lsAssert(target.isWhite == ret.isWhitesTurn);
      }
      else // en passant
      {
        assert_move_type(move, cmt_pawn_en_passant, board);
        const vec2i8 enemyPos = vec2i8(move.targetX, move.startY);
        lsAssert(board[enemyPos].piece && board[enemyPos].lastWasDoubleStep && board[enemyPos].piece == cpT_pawn && (board[enemyPos].isWhite == ret.isWhitesTurn));
        ret[enemyPos].piece = cpT_none;
      }
    }
    else if (move.isPromotion)
    {
      assert_move_type(move, cmt_pawn_promotion, board);
      lsAssert((board.isWhitesTurn && move.targetY == BoardWidth - 1) || (!board.isWhitesTurn && move.targetY == 0));

      if (move.isPromotedToQueen)
        origin.piece = cpT_queen;
      else
        origin.piece = cpT_knight;
    }
    else
    {
      assert_move_type(move, cmt_pawn, board);
    }
  }
  else if (origin.piece == cpT_king)
  {
    // castlen
    if (lsAbs(move.startX - move.targetX) > 1)
    {
      assert_move_type(move, cmt_king_castle, board);
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
    else
    {
      assert_move_type(move, cmt_king, board);
    }
  }
  else if (origin.piece == cpT_knight)
  {
    assert_move_type(move, cmt_knight, board);
  }
  else if (origin.piece == cpT_bishop)
  {
    assert_move_type(move, cmt_bishop, board);
  }
  else if (origin.piece == cpT_rook)
  {
    assert_move_type(move, cmt_rook, board);
  }
  else if (origin.piece == cpT_queen)
  {
    if (move.startX != move.targetX && move.startY != move.targetY)
      assert_move_type(move, cmt_queen_diagonal, board);
    else
      assert_move_type(move, cmt_queen_straight, board);
  }

  origin.hasMoved = true;
  target = std::move(origin);

  return ret;
}

//////////////////////////////////////////////////////////////////////////

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

//////////////////////////////////////////////////////////////////////////

constexpr uint8_t SquareWeightFactor = 1;

struct square_weights
{
  int8_t weights[BoardWidth * BoardWidth];

  constexpr square_weights(const std::initializer_list<int8_t> &w, const uint8_t scale = SquareWeightFactor)
  {
    lsAssert(w.size() == LS_ARRAYSIZE(weights));

    size_t i = (size_t)-1;

    for (const int8_t v : w)
    {
      ++i;
      lsAssert((int64_t)v * (int64_t)scale <= lsMaxValue<int8_t>() && (int64_t)v * (int64_t)scale >= lsMinValue<int8_t>());
      weights[i] = v * (int8_t)scale;
    }
  }
};

static const square_weights SquareWeights[] = { // https://www.talkchess.com/forum/viewtopic.php?p=553266#p553266
  square_weights({ // none
  0,   0,   0,   0,   0,   0,   0,   0,
  0,   0,   0,   0,   0,   0,   0,   0,
  0,   0,   0,   0,   0,   0,   0,   0,
  0,   0,   0,   0,   0,   0,   0,   0,
  0,   0,   0,   0,   0,   0,   0,   0,
  0,   0,   0,   0,   0,   0,   0,   0,
  0,   0,   0,   0,   0,   0,   0,   0,
  0,   0,   0,   0,   0,   0,   0,   0
  }),
  square_weights({ // king
  -40, -33, -26, -20, -20, -26, -33, -40,
  -33, -26, -20, -13, -13, -20, -26, -33,
  -26, -20, -13,  -7,  -7, -13, -20, -26,
  -20, -13,  -7,   0,   0,  -7, -13, -20,
  -20, -13,  -7,   0,   0,  -7, -13, -20,
  -26, -20, -13,  -7,  -7, -13, -20, -26,
  -23, -16, -10,  -3,  -3, -10, -16, -23,
  -25, -13,  -6,  -5,   0, -11, -13, -25
  }),
  square_weights({ // queen
  0,   0,   0,   0,   0,   0,   0,   0,
  0,   0,   0,   0,   0,   0,   0,   0,
  0,   0,   0,   0,   0,   0,   0,   0,
  0,   0,   0,   0,   0,   0,   0,   0,
  0,   0,   0,   0,   0,   0,   0,   0,
  0,   0,   0,   0,   0,   0,   0,   0,
  0,   0,   0,   0,   0,   0,   0,   0,
  0,   0,   0,   0,   0,   0,   0,   0
  }),
  square_weights({ // rook
  0,  0,  0,  0,  0,  0,  0,  0,
  20, 20, 20, 20, 20, 20, 20, 20,
  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0
  }),
  square_weights({ // bishop
  -10,  -6,  -3,   0,   0,  -3,  -6, -10,
   -6,  -3,   0,   3,   3,   0,  -3,  -6,
   -3,   0,   3,   6,   6,   3,   0,  -3,
    0,   3,   6,  10,  10,   6,   3,   0,
    0,   3,   6,  10,  10,   6,   3,   0,
   -3,   0,   3,   6,   6,   3,   0,  -3,
   -6,  -3,   0,   3,   3,   0,  -3,  -6,
  -10,  -6,  -3,   0,   0,  -3,  -6, -10
  }),
  square_weights({ // knight
  -10,  -6,  -3,   0,   0,  -3,  -6, -10,
   -6,  -3,   0,   3,   3,   0,  -3,  -6,
   -3,   0,   3,   6,   6,   3,   0,  -3,
    0,   3,   6,  10,  10,   6,   3,   0,
    0,   3,   6,  10,  10,   6,   3,   0,
   -3,   0,   3,   6,   6,   3,   0,  -3,
   -6,  -3,   0,   3,   3,   0,  -3,  -6,
  -10,  -6,  -3,   0,   0,  -3,  -6, -10
  }),
  square_weights({ // pawn
  0,   0,   0,   0,   0,   0,   0,   0,
  0,   0,   0,   0,   0,   0,   0,   0,
  0,   0,   0,  10,  10,   0,   0,   0,
  0,   0,   5,  10,  10,   5,   0,   0,
  0,   0,   5,  10,  10,   5,   0,   0,
  0,   0,   5,   5,   5,   5,   0,   0,
  0,   0,   0,   0,   0,   0,   0,   0,
  0,   0,   0,   0,   0,   0,   0,   0
  })
};

static_assert(LS_ARRAYSIZE(SquareWeights) == _chess_piece_type_count);

int64_t evaluate_chess_board(const chess_board &board)
{
  constexpr int64_t PieceScores[] = { 0, 100000, 950, 563, 333, 305, 100 }; // Chess piece values from `https://en.wikipedia.org/wiki/Chess_piece_relative_value#Alternative_valuations > AlphaZero`.

  int64_t ret = 0;

  for (size_t i = 0; i < LS_ARRAYSIZE(board.board); i++)
  {
    const chess_piece p = board.board[i];
    const int64_t negate = !p.isWhite;

    const uint64_t posMaskBlack = (uint64_t)p.isWhite - 1; // 0 -> All Bits, 1 -> No Bits
    const uint64_t posWhite = i;
    const uint64_t posBlack = (((LS_ARRAYSIZE(board.board) - 1) - i) & ~7) | (i & 7); // invert y.
    const uint64_t pos = (posMaskBlack & posBlack) | ((~posMaskBlack) & posWhite); // branchless select.

    const int64_t squareScore = SquareWeights[p.piece].weights[pos];
    const int64_t pieceScore = PieceScores[p.piece];
    const int64_t score = squareScore + pieceScore;

    const int64_t signedScore = ((score) ^ -negate) + negate;
    const uint64_t scoreZeroMask = (uint64_t)(!score) - 1;
    const int64_t signedScoreWithZero = (int64_t)((uint64_t)signedScore & scoreZeroMask);

    ret += signedScoreWithZero; // https://graphics.stanford.edu/~seander/bithacks.html#ConditionalNegate
  }

  //if (!board.isWhitesTurn)
    //ret = -ret;

  return ret;
}

struct move_with_score
{
  chess_move move;
  int64_t score;

  move_with_score() = default;
  move_with_score(const chess_move move, const int64_t score) : move(move), score(score) {}
};

template <size_t DepthRemaining, bool FindMin>
move_with_score minimax_step(const chess_board &board)
{
  if constexpr (DepthRemaining == 0)
  {
    return move_with_score({}, evaluate_chess_board(board));
  }
  else
  {
    small_list<chess_move> moves;
    LS_DEBUG_ERROR_ASSERT(get_all_valid_moves(board, moves));
    move_with_score ret;
    ret.move = {};
    ret.score = FindMin ? lsMaxValue<int64_t>() : lsMinValue<int64_t>();

    for (const chess_move move : moves)
    {
      const chess_board after = perform_move(board, move);
      const move_with_score move_rating = minimax_step<DepthRemaining - 1, !FindMin>(after);

      if constexpr (FindMin)
      {
        if (move_rating.score < ret.score)
          ret = move_with_score(move, move_rating.score);
      }
      else
      {
        if (move_rating.score > ret.score)
          ret = move_with_score(move, move_rating.score);
      }
    }

    return ret;
  }
}

template <size_t MaxDepth>
struct alpha_beta_minimax_cache
{
  small_list<chess_move> movesAtLevel[MaxDepth];
#ifdef _DEBUG
  size_t nodesVisited = 0;
  size_t duplicatesRejected = 0;
#endif

  constexpr static size_t hashBits = 20;
  constexpr static size_t hashValues = (1ULL << hashBits);
  constexpr static size_t hashMask = hashValues - 1;

  chess_hash_board *pCache = nullptr;

  ~alpha_beta_minimax_cache()
  {
    lsFreePtr(&pCache);
  }
};

constexpr float_t CacheSizeMB = (alpha_beta_minimax_cache<1>::hashValues * sizeof(chess_hash_board)) / (1024.0 * 1024.0);

template <size_t MaxDepth>
lsResult alpha_beta_minimax_cache_create(alpha_beta_minimax_cache<MaxDepth> &cache)
{
  return lsAllocZero(&cache.pCache, cache.hashValues);
}

template <size_t MaxDepth>
bool alpha_beta_minimax_cache_find(const alpha_beta_minimax_cache<MaxDepth> &cache, const chess_hash_board &board, uint64_t &outHash, move_with_score &outRet)
{
  const uint64_t hash = lsHash(board) & cache.hashMask;
  const chess_hash_board found = cache.pCache[hash];

  outHash = hash;
  outRet = move_with_score(found.move, found.score);

  return found == board;
}

template <size_t MaxDepth>
void alpha_beta_minimax_cache_store(alpha_beta_minimax_cache<MaxDepth> &cache, chess_hash_board &board, const uint64_t hash, const move_with_score ret)
{
  board.score = (int16_t)ret.score;
  board.move = ret.move;
  cache.pCache[hash] = std::move(board);
}

template <size_t DepthRemaining, bool FindMin, size_t MaxDepth>
move_with_score alpha_beta_step(const chess_board &board, int64_t alpha, int64_t beta, alpha_beta_minimax_cache<MaxDepth> &cache)
{
  static_assert(DepthRemaining <= MaxDepth);

  chess_hash_board hashBoard = chess_hash_board_create(board);
  uint64_t hash;

  if constexpr (DepthRemaining == 0)
  {
    const move_with_score ret = move_with_score({}, evaluate_chess_board(board));
    //alpha_beta_minimax_cache_store(cache, hashBoard, hash, ret);
    return ret;
  }
  else
  {
    {
      move_with_score ret;

      if (alpha_beta_minimax_cache_find(cache, hashBoard, hash, ret))
      {
        cache.duplicatesRejected++;
        return ret;
      }
    }

    small_list<chess_move> &moves = cache.movesAtLevel[DepthRemaining - 1];
    list_clear(&moves);

    LS_DEBUG_ERROR_ASSERT(get_all_valid_moves(board, moves));
    move_with_score ret;
    ret.move = {};
    ret.score = FindMin ? lsMaxValue<int64_t>() : -lsMaxValue<int64_t>();  // -lsMinValue<int64_t>() would not be depictable

    for (const chess_move move : moves)
    {
#ifdef _DEBUG
      cache.nodesVisited++;
#endif

      const chess_board after = perform_move(board, move);
      const move_with_score moveRating = alpha_beta_step<DepthRemaining - 1, !FindMin, MaxDepth>(after, alpha, beta, cache);

      if constexpr (FindMin)
      {
        if (moveRating.score < ret.score)
        {
          ret = move_with_score(move, moveRating.score);

          if (ret.score < beta)
            beta = ret.score;

          if (ret.score <= alpha)
            break;
        }
      }
      else
      {
        if (moveRating.score > ret.score)
        {
          ret = move_with_score(move, moveRating.score);

          if (ret.score > alpha)
            alpha = ret.score;

          if (ret.score >= beta)
            break;
        }
      }
    }

    alpha_beta_minimax_cache_store(cache, hashBoard, hash, ret);
    return ret;
  }
}

constexpr size_t DefaultMinimaxDepth = 4;

chess_move get_minimax_move_white(const chess_board &board)
{
  const move_with_score moveInfo = minimax_step<DefaultMinimaxDepth, true>(board);
  return moveInfo.move;
}

chess_move get_minimax_move_black(const chess_board &board)
{
  const move_with_score moveInfo = minimax_step<DefaultMinimaxDepth, false>(board);
  return moveInfo.move;
}

constexpr size_t DefaultAlphaBetaDepth = 6;

template <bool IsWhite>
chess_move get_alpha_beta_move(const chess_board &board)
{
#ifdef _DEBUG
  const int64_t before = lsGetCurrentTimeNs();
#endif

  alpha_beta_minimax_cache<DefaultAlphaBetaDepth> cache;
  LS_DEBUG_ERROR_ASSERT(alpha_beta_minimax_cache_create(cache));

  const move_with_score moveInfo = alpha_beta_step<DefaultAlphaBetaDepth, !IsWhite>(board, lsMinValue<int64_t>(), lsMaxValue<int64_t>(), cache);

#ifdef _DEBUG
  const int64_t after = lsGetCurrentTimeNs();

  print(FU(Group)(cache.nodesVisited), " nodes visited, ", FU(Group)(cache.duplicatesRejected), " duplicates rejected (in ", FF(Max(5))((after - before) * 1e-9f), "s, ", FF(Max(9), Group)(cache.nodesVisited / ((after - before) * 1e-9f)), "/s). Final score: ", moveInfo.score, '\n');
#endif

  return moveInfo.move;
}

chess_move get_alpha_beta_white(const chess_board &board)
{
  return get_alpha_beta_move<true>(board);
}

chess_move get_alpha_beta_black(const chess_board &board)
{
  return get_alpha_beta_move<false>(board);
}

//////////////////////////////////////////////////////////////////////////

void print_move(const chess_move move)
{
  print((char)(move.startX + 'a'), move.startY + 1, (char)(move.targetX + 'a'), move.targetY + 1);
}

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

REGISTER_TESTABLE_FILE(0)

DEFINE_TESTABLE(pawn_double_step_test)
{
  lsResult result = lsR_Success;

  chess_board board = chess_board::get_starting_point();
  print_board(board);
  board = perform_move(board, chess_move(vec2i8(4, 1), vec2i8(4, 3), cmt_pawn_double_step));
  print_board(board);

  board = perform_move(board, chess_move(vec2i8(1, 6), vec2i8(1, 4), cmt_pawn_double_step));
  print_board(board);

  TESTABLE_ASSERT_FALSE(!!board[vec2i8(4, 3)].lastWasDoubleStep);

  board = perform_move(board, chess_move(vec2i8(0, 1), vec2i8(0, 3), cmt_pawn_double_step));
  print_board(board);
  board = perform_move(board, chess_move(vec2i8(1, 4), vec2i8(1, 3), cmt_pawn));
  print_board(board);
  board = perform_move(board, chess_move(vec2i8(0, 0), vec2i8(0, 2), cmt_rook));
  print_board(board);

  TESTABLE_ASSERT_FALSE(!!board[vec2i8(0, 3)].lastWasDoubleStep);

  goto epilogue;
epilogue:
  return result;
}
