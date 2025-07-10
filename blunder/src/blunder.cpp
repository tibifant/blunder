#include "blunder.h"
#include "testable.h"
#include "local_list.h"

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

  if (is_check_straight(board, pos, isWhite, vec2i8(0, 1)) || is_check_straight(board, pos, isWhite, vec2i8(0, -1)) || is_check_straight(board, pos, isWhite, vec2i8(1, 0)) || is_check_straight(board, pos, isWhite, vec2i8(-1, 0)) || is_check_diagonal(board, pos, isWhite, vec2i8(1, 1)) || is_check_diagonal(board, pos, isWhite, vec2i8(-1, -1)))
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

template <typename T>
struct result_of;

template <typename TRet, typename ...TArgs>
struct result_of<TRet(TArgs...)>
{
  using type = TRet;
};

template <typename T>
using result_of_t = result_of<T>::type;

template <auto TFunc, auto TResultNop, typename TParam>
concept TFuncIsValid = requires (const decltype(TResultNop) ret, TParam & param, const chess_move & move, const chess_board & board) {
  { TFunc(param, move, board) } -> std::same_as<decltype(TResultNop)>;
  { is_cancel(ret) } -> std::same_as<bool>;
};

//////////////////////////////////////////////////////////////////////////

template <auto TFunc, auto TResultNop, typename TParam>
  requires TFuncIsValid<TFunc, TResultNop, TParam>
auto add_valid_move(const vec2i8 origin, const vec2i8 destination, const chess_board &board, TParam &param, [[maybe_unused]] const chess_move_type type)
{
  if (destination.x >= 0 && destination.x < BoardWidth && destination.y >= 0 && destination.y < BoardWidth && (!board[destination].piece || board[destination].isWhite != board.isWhitesTurn))
    return TFunc(param, chess_move(origin, destination, type), board);
  else
    return TResultNop;
}

template <auto TFunc, auto TResultNop, typename TParam>
  requires TFuncIsValid<TFunc, TResultNop, TParam>
auto add_valid_move(const chess_move move, const chess_board &board, TParam &param)
{
  const vec2i8 dest = vec2i8(move.targetX, move.targetY);

  if (move.targetX >= 0 && move.targetX < BoardWidth && move.targetY >= 0 && move.targetY < BoardWidth && (!board[dest].piece || board[dest].isWhite != board.isWhitesTurn))
    return TFunc(param, move, board);
  else
    return TResultNop;
}

template <auto TFunc, auto TResultNop, typename TParam>
  requires TFuncIsValid<TFunc, TResultNop, TParam>
inline auto add_repeated_moves(const chess_board &board, const vec2i8 startPos, const vec2i8 dir, TParam &param, [[maybe_unused]] const chess_move_type type)
{
  auto result = TResultNop;

  lsAssert(startPos.x >= 0 && startPos.x < BoardWidth && startPos.y >= 0 && startPos.y < BoardWidth);

  for (int8_t i = 1; i < BoardWidth; i++)
  {
    const vec2i8 targetPos = startPos + vec2i8(i) * dir;

    if (targetPos.x >= 0 && targetPos.x < BoardWidth && targetPos.y >= 0 && targetPos.y < BoardWidth)
    {
      if (is_cancel(result = add_valid_move<TFunc, TResultNop, TParam>(startPos, targetPos, board, param, type)))
        return result;

      if (board[targetPos].piece)
        break;
    }
  }

  return result;
}

template <auto TFunc, auto TResultNop, typename TParam>
  requires TFuncIsValid<TFunc, TResultNop, TParam>
inline auto add_potential_promotion(chess_move move, const chess_board &board, TParam &param)
{
  auto result = TResultNop;

  if (move.targetY == BoardWidth - 1 || move.targetY == 0)
  {
#ifdef _DEBUG
    move.moveType = cmt_pawn_promotion;
#endif
    move.isPromotion = true;
    move.isPromotedToQueen = true;

    if (is_cancel(result = add_valid_move<TFunc, TResultNop, TParam>(move, board, param)))
      return result;

    move.isPromotedToQueen = false;

    if (is_cancel(result = add_valid_move<TFunc, TResultNop, TParam>(move, board, param)))
      return result;
  }
  else
  {
    if (is_cancel(result = add_valid_move<TFunc, TResultNop, TParam>(move, board, param)))
      return result;
  }

  return result;
}

template <auto TFunc, auto TResultNop, typename TParam>
  requires TFuncIsValid<TFunc, TResultNop, TParam>
inline auto get_pawn_moves_from(const chess_board &board, TParam &param, const vec2i8 startPos)
{
  auto result = TResultNop;

  const vec2i8 dir = board[startPos].isWhite ? vec2i8(0, 1) : vec2i8(0, -1);
  const vec2i8 targetPos = vec2i8(startPos + dir);
  const vec2i8 doubleStepTargetPos = targetPos + dir;
  const vec2i8 diagonalLeftTargetPos = vec2i8(targetPos.x - 1, targetPos.y);
  const vec2i8 diagonalRightTargetPos = vec2i8(targetPos.x + 1, targetPos.y);

  if (targetPos.y < BoardWidth && targetPos.y >= 0 && !board[targetPos].piece)
  {
    if (((board.isWhitesTurn && startPos.y == 1) || (!board[startPos].isWhite && startPos.y == 6)) && !board[doubleStepTargetPos].piece)
    {
      if (is_cancel(result = add_valid_move<TFunc, TResultNop, TParam>(startPos, doubleStepTargetPos, board, param, cmt_pawn_double_step)))
        return result;
    }

    chess_move move = chess_move(startPos, targetPos, cmt_pawn);

    if (is_cancel(result = add_potential_promotion<TFunc, TResultNop, TParam>(move, board, param)))
      return result;
  }

  if (diagonalLeftTargetPos.x >= 0 && diagonalLeftTargetPos.y >= 0 && diagonalLeftTargetPos.y < BoardWidth)
  {
    const chess_piece enemyPiece = board[diagonalLeftTargetPos];

    if (enemyPiece.piece && (enemyPiece.isWhite != board.isWhitesTurn))
    {
      chess_move move = chess_move(startPos, diagonalLeftTargetPos, cmt_pawn_capture);

      if (is_cancel(result = add_potential_promotion<TFunc, TResultNop, TParam>(move, board, param)))
        return result;
    }
  }

  if (diagonalRightTargetPos.x < BoardWidth && diagonalRightTargetPos.y >= 0 && diagonalRightTargetPos.y < BoardWidth)
  {
    const chess_piece enemyPiece = board[diagonalRightTargetPos];

    if (enemyPiece.piece && (enemyPiece.isWhite != board.isWhitesTurn))
    {
      chess_move move = chess_move(startPos, diagonalRightTargetPos, cmt_pawn_capture);

      if (is_cancel(result = add_potential_promotion<TFunc, TResultNop, TParam>(move, board, param)))
        return result;
    }
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
        if (is_cancel(result = add_valid_move<TFunc, TResultNop, TParam>(startPos, targetPosLeft, board, param, cmt_pawn_en_passant)))
          return result;
    }

    if (enemyPosRight.x < BoardWidth)
    {
      const chess_piece enemyPiece = board[enemyPosRight];
      const vec2i8 targetPosRight = vec2i8(targetPos.x + 1, targetPos.y);

      if (enemyPiece.piece == cpT_pawn && enemyPiece.lastWasDoubleStep && (enemyPiece.isWhite != board.isWhitesTurn))
        if (is_cancel(result = add_valid_move<TFunc, TResultNop, TParam>(startPos, targetPosRight, board, param, cmt_pawn_en_passant)))
          return result;
    }
  }

  return result;
}

template <auto TFunc, auto TResultNop, typename TParam>
  requires TFuncIsValid<TFunc, TResultNop, TParam>
inline auto add_castle_moves_from(const chess_board &board, TParam &param, const vec2i8 kingStartPos)
{
  auto result = TResultNop;

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
        if (is_cancel(result = TFunc(param, chess_move(kingStartPos, rookPosLeft + vec2i8(1, 0), cmt_king_castle), board))) // all checks from `add_valid_move` have already been checked
          return result;
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
        if (is_cancel(result = TFunc(param, chess_move(kingStartPos, rookPosRight + vec2i8(-1, 0), cmt_king_castle), board))) // all checks from `add_valid_move` have already been checked
          return result;
    }
  }

  return result;
}

template <chess_piece_type piece, auto TFunc, auto TResultNop, typename TParam>
  requires TFuncIsValid<TFunc, TResultNop, TParam>
auto get_all_valid_piece_moves(const chess_board &board, TParam &param)
{
  auto result = TResultNop;

  for (int8_t y = 0; y < BoardWidth; y++)
  {
    for (int8_t x = 0; x < BoardWidth; x++)
    {
      const vec2i8 startPos = vec2i8(x, y);

      if (board[startPos].piece == piece && board[startPos].isWhite == board.isWhitesTurn)
      {
        if constexpr (piece == cpT_pawn)
        {
          if (is_cancel(result = get_pawn_moves_from<TFunc, TResultNop, TParam>(board, param, startPos)))
            return result;
        }
        else if constexpr (piece == cpT_knight)
        {
          constexpr static vec2i8 TargetDir[] = { vec2i8(-2, -1), vec2i8(-1, -2), vec2i8(1, -2), vec2i8(2, -1), vec2i8(2, 1), vec2i8(1, 2), vec2i8(-1, 2), vec2i8(-2, 1) };

          for (size_t i = 0; i < LS_ARRAYSIZE(TargetDir); i++)
            if (is_cancel(result = add_valid_move<TFunc, TResultNop, TParam>(startPos, vec2i8(startPos + TargetDir[i]), board, param, cmt_knight)))
              return result;
        }
        else if constexpr (piece == cpT_bishop || piece == cpT_rook || piece == cpT_queen)
        {
          if constexpr (piece != cpT_rook)
          {
            constexpr chess_move_type moveType = piece == cpT_bishop ? cmt_bishop : cmt_queen_diagonal;
            if (is_cancel(result = add_repeated_moves<TFunc, TResultNop, TParam>(board, startPos, TopLeftRelative, param, moveType)) ||
              is_cancel(result = add_repeated_moves<TFunc, TResultNop, TParam>(board, startPos, TopRightRelative, param, moveType)) ||
              is_cancel(result = add_repeated_moves<TFunc, TResultNop, TParam>(board, startPos, BottomLeftRelative, param, moveType)) ||
              is_cancel(result = add_repeated_moves<TFunc, TResultNop, TParam>(board, startPos, BottomRightRelative, param, moveType)))
              return result;
          }

          if constexpr (piece != cpT_bishop)
          {
            constexpr chess_move_type moveType = piece == cpT_rook ? cmt_rook : cmt_queen_straight;
            if (is_cancel(result = add_repeated_moves<TFunc, TResultNop, TParam>(board, startPos, LeftRelative, param, moveType)) ||
              is_cancel(result = add_repeated_moves<TFunc, TResultNop, TParam>(board, startPos, TopRelative, param, moveType)) ||
              is_cancel(result = add_repeated_moves<TFunc, TResultNop, TParam>(board, startPos, RightRelative, param, moveType)) ||
              is_cancel(result = add_repeated_moves<TFunc, TResultNop, TParam>(board, startPos, BottomRelative, param, moveType)))
              return result;
          }
        }
        else if constexpr (piece == cpT_king)
        {
          constexpr vec2i8 TargetDir[] = { TopLeftRelative, TopRelative, TopRightRelative, LeftRelative, RightRelative, BottomLeftRelative, BottomRelative, BottomRightRelative };

          for (size_t i = 0; i < LS_ARRAYSIZE(TargetDir); i++)
            if (is_cancel(result = add_valid_move<TFunc, TResultNop, TParam>(startPos, vec2i8(startPos + TargetDir[i]), board, param, cmt_king)))
              return result;

          if (is_cancel(result = add_castle_moves_from<TFunc, TResultNop, TParam>(board, param, startPos)))
            return result;
        }
        else
        {
          static_assert(piece == cpT_none);
          lsFail();
        }
      }
    }
  }

  return result;
}

template <auto TFunc, auto TResultNop, typename TParam>
  requires TFuncIsValid<TFunc, TResultNop, TParam>
auto get_all_valid_moves(const chess_board &board, TParam &param)
{
  auto result = TResultNop;

  if (is_cancel(result = get_all_valid_piece_moves<cpT_pawn, TFunc, TResultNop, TParam>(board, param)) ||
    is_cancel(result = get_all_valid_piece_moves<cpT_king, TFunc, TResultNop, TParam>(board, param)) ||
    is_cancel(result = get_all_valid_piece_moves<cpT_queen, TFunc, TResultNop, TParam>(board, param)) ||
    is_cancel(result = get_all_valid_piece_moves<cpT_rook, TFunc, TResultNop, TParam>(board, param)) ||
    is_cancel(result = get_all_valid_piece_moves<cpT_bishop, TFunc, TResultNop, TParam>(board, param)) ||
    is_cancel(result = get_all_valid_piece_moves<cpT_knight, TFunc, TResultNop, TParam>(board, param)))
    return result;

  return result;
}

//////////////////////////////////////////////////////////////////////////

__forceinline bool is_cancel(const lsResult result)
{
  return LS_FAILED(result);
}

__forceinline lsResult list_add_adapter(list<chess_move> &moves, const chess_move &move, const chess_board &)
{
  return list_add(&moves, move);
}

//////////////////////////////////////////////////////////////////////////

lsResult get_all_valid_moves(const chess_board &board, list<chess_move> &moves)
{
  list_clear(&moves);

  return get_all_valid_moves<list_add_adapter, lsR_Success, list<chess_move>>(board, moves);
}

//////////////////////////////////////////////////////////////////////////

struct capture_info_chess_move : chess_move
{
  chess_piece_type capturingPiece;
  chess_piece_type capturedPiece;

  capture_info_chess_move(const chess_move move, const chess_piece_type capturingPiece, const chess_piece_type capturedPiece) : capturingPiece(capturingPiece), capturedPiece(capturedPiece), chess_move(move) {}
};

template <bool HasCptNone>
struct piece_move_map
{
  list<capture_info_chess_move> map[_chess_piece_type_count - (uint8_t)!HasCptNone];
};

template <bool IsQuiescence>
lsResult add_capturing_move(piece_move_map<false> &map, const chess_move &move, const chess_board &board)
{
  const chess_piece_type capturingPiece = board[vec2i8(move.startX, move.startY)].piece;
  const chess_piece_type capturedPiece = board[vec2i8(move.targetX, move.targetY)].piece;

  if constexpr (IsQuiescence)
    if (!capturedPiece)
      return lsR_Success;

  lsAssert(capturingPiece - 1 <= LS_ARRAYSIZE(map.map));
  return list_add(map.map[capturingPiece - 1], capture_info_chess_move(move, capturingPiece, capturedPiece));
}

template <bool HasCptNoneIn, bool HasCptNoneTmp>
lsResult retrieve_ordered_moves(list<chess_move> &out, const piece_move_map<HasCptNoneIn> &in, piece_move_map<HasCptNoneTmp> &tmp)
{
  lsResult result = lsR_Success;

  list_clear(&out);

  for (size_t i = 0; i < LS_ARRAYSIZE(tmp.map); i++)
    list_clear(&tmp.map[i]);

  for (int64_t i = LS_ARRAYSIZE(in.map) - 1; i >= 0; i--)
    for (const capture_info_chess_move m : in.map[i])
      LS_ERROR_CHECK(list_add(tmp.map[m.capturedPiece - !HasCptNoneTmp], m));

  // step 1: move capturing pieces
  for (size_t i = cpT_none + 1; i < LS_ARRAYSIZE(tmp.map); i++) // relies on the chess_pice_types being in the right order
    for (const capture_info_chess_move m : tmp.map[i - !HasCptNoneTmp])
      LS_ERROR_CHECK(list_add<chess_move>(out, m));

  // step 2: if there could be non-capturing moves: put them last.
  if constexpr (HasCptNoneTmp)
    for (const capture_info_chess_move m : tmp.map[cpT_none])
      LS_ERROR_CHECK(list_add<chess_move>(out, m));

epilogue:
  return result;
}

//////////////////////////////////////////////////////////////////////////

lsResult get_valid_quiescence_moves(list<chess_move> &out, const chess_board &board, piece_move_map<false> &in, piece_move_map<false> &tmp)
{
  lsResult result = lsR_Success;

  for (size_t i = 0; i < LS_ARRAYSIZE(in.map); i++)
    list_clear(&in.map[i]);

  LS_ERROR_CHECK((get_all_valid_moves<add_capturing_move<true>, lsR_Success, piece_move_map<false>>(board, in)));
  LS_ERROR_CHECK(retrieve_ordered_moves(out, in, tmp));

epilogue:
  return result;
}

lsResult get_all_valid_ordered_moves(list<chess_move> &out, const chess_board &board, piece_move_map<false> &in, piece_move_map<true> &tmp)
{
  lsResult result = lsR_Success;

  for (size_t i = 0; i < LS_ARRAYSIZE(in.map); i++)
    list_clear(&in.map[i]);

  LS_ERROR_CHECK((get_all_valid_moves<add_capturing_move<false>, lsR_Success, piece_move_map<false>>(board, in)));
  LS_ERROR_CHECK(retrieve_ordered_moves(out, in, tmp));

epilogue:
  return result;
}

//////////////////////////////////////////////////////////////////////////

bool is_upper_case(const char c)
{
  return 'A' <= c && c <= 'Z';
}

chess_board get_board_from_starting_position(const char *startingPosition)
{
  chess_board ret;

  vec2i8 currentPos = vec2i8(0, 7);
  size_t i = (size_t)(-1);

  while (true)
  {
    i++;

    chess_piece_type piece = cpT_none;
    bool isWhite = false;

    switch (startingPosition[i])
    {
    case '.':
    case ' ':
      piece = cpT_none;
      break;

    case 'K':
    case 'k':
      piece = cpT_king;
      isWhite = is_upper_case(startingPosition[i]);
      break;

    case 'Q':
    case 'q':
      piece = cpT_queen;
      isWhite = is_upper_case(startingPosition[i]);
      break;

    case 'N':
    case 'n':
      piece = cpT_knight;
      isWhite = is_upper_case(startingPosition[i]);
      break;

    case 'B':
    case 'b':
      piece = cpT_bishop;
      isWhite = is_upper_case(startingPosition[i]);
      break;

    case 'R':
    case 'r':
      piece = cpT_rook;
      isWhite = is_upper_case(startingPosition[i]);
      break;

    case 'P':
    case 'p':
      piece = cpT_pawn;
      isWhite = is_upper_case(startingPosition[i]);
      break;

    case '\r':
      continue;

    case '\n':
      currentPos.x = 0;
      currentPos.y--;
      continue;

    default:
      print_error_line("Unexpected Token in stream: ", startingPosition[i]);
      lsFail();
    }

    lsAssert(currentPos.x < BoardWidth && currentPos.y < BoardWidth);

    ret[lsMin(currentPos, vec2i8(BoardWidth - 1, BoardWidth - 1))] = chess_piece(piece, isWhite);
    currentPos.x++;

    if ((currentPos.x == 8 && currentPos.y == 0) || currentPos.y < 0)
      break;
  }

  const chess_board startBoard = chess_board::get_starting_point();

  for (size_t j = 0; j < LS_ARRAYSIZE(startBoard.board); j++)
    if (ret.board[j].piece != startBoard.board[j].piece)
      ret.board[j].hasMoved = true;

  return ret;
}

chess_board get_board_from_fen(const char *fenString)
{
  chess_board ret;

  vec2i8 currentPos = vec2i8(0, 7);
  size_t i = (size_t)(-1);

  while (true)
  {
    i++;

    chess_piece_type piece = cpT_none;
    bool isWhite = false;

    switch (fenString[i])
    {
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    {
      const uint8_t count = fenString[i] - '0';
      lsAssert(currentPos.x + count - 1 < BoardWidth);

      for (size_t j = 0; j < count; j++)
      {
        ret[currentPos] = chess_piece(cpT_none, false);
        currentPos.x++;
      }

      continue;
    }

    case 'K':
    case 'k':
      piece = cpT_king;
      isWhite = is_upper_case(fenString[i]);
      break;

    case 'Q':
    case 'q':
      piece = cpT_queen;
      isWhite = is_upper_case(fenString[i]);
      break;

    case 'N':
    case 'n':
      piece = cpT_knight;
      isWhite = is_upper_case(fenString[i]);
      break;

    case 'B':
    case 'b':
      piece = cpT_bishop;
      isWhite = is_upper_case(fenString[i]);
      break;

    case 'R':
    case 'r':
      piece = cpT_rook;
      isWhite = is_upper_case(fenString[i]);
      break;

    case 'P':
    case 'p':
      piece = cpT_pawn;
      isWhite = is_upper_case(fenString[i]);
      break;

    case '/':
      currentPos.x = 0;
      currentPos.y--;
      continue;

    default:
      print_error_line("Unexpected Token in stream: ", fenString[i]);
      lsFail();
    }

    lsAssert(currentPos.x < BoardWidth && currentPos.y < BoardWidth);

    ret[lsMin(currentPos, vec2i8(BoardWidth - 1, BoardWidth - 1))] = chess_piece(piece, isWhite);
    currentPos.x++;

    if ((currentPos.x == 8 && currentPos.y == 0) || currentPos.y < 0)
      break;
  }

  i++;
  lsAssert(fenString[i] == ' ');

  i++;
  ret.isWhitesTurn = fenString[i] == 'w';

  const chess_board startBoard = chess_board::get_starting_point();

  for (size_t j = 0; j < LS_ARRAYSIZE(startBoard.board); j++)
    if (ret.board[j].piece != startBoard.board[j].piece)
      ret.board[j].hasMoved = true;

  return ret;
}

//////////////////////////////////////////////////////////////////////////

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
  __m128i v0 = _mm_loadu_si128(reinterpret_cast<const __m128i *>(board.nibbleMap));
  __m128i v1 = _mm_loadu_si128(reinterpret_cast<const __m128i *>(board.nibbleMap) + 1);
  v0 = _mm_aesdec_si128(v0, v1);
  uint64_t ret = _mm_extract_epi64(v0, 0);
  ret ^= board.isWhitesTurn;
  return ret;
}

simple_chess_hash_board simple_chess_hash_board_create(const chess_board &board)
{
  simple_chess_hash_board ret;
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

uint64_t lsHash(const simple_chess_hash_board &board)
{
  __m128i v0 = _mm_loadu_si128(reinterpret_cast<const __m128i *>(board.nibbleMap));
  __m128i v1 = _mm_loadu_si128(reinterpret_cast<const __m128i *>(board.nibbleMap) + 1);
  v0 = _mm_aesdec_si128(v0, v1);
  uint64_t ret = _mm_extract_epi64(v0, 0);
  ret ^= board.isWhitesTurn;
  return ret;
}

//////////////////////////////////////////////////////////////////////////

__forceinline void assert_move_type(const chess_move move, const chess_move_type type, [[maybe_unused]] const chess_board &board)
{
#ifdef _DEBUG
  lsAssert(move.moveType == type);
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
    else if (move.isPromotion)
    {
      assert_move_type(move, cmt_pawn_promotion, board);
      lsAssert((board.isWhitesTurn && move.targetY == BoardWidth - 1) || (!board.isWhitesTurn && move.targetY == 0));

      if (move.isPromotedToQueen)
        origin.piece = cpT_queen;
      else
        origin.piece = cpT_knight;
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

static const square_weights SquareWeights[] = {
  square_weights({ // none
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0
  }),
  square_weights({ // king
    20, 30, 10, 0, 0, 10, 30, 20,
    20, 20, 0, 0, 0, 0, 20, 20,
    -10, -20, -20, -20, -20, -20, -20, -10,
    -20, -30, -30, -40, -40, -30, -30, -20,
    -30, -40, -40, -50, -50, -40, -40, -30,
    -30, -40, -40, -50, -50, -40, -40, -30,
    -30, -40, -40, -50, -50, -40, -40, -30,
    -30, -40, -40, -50, -50, -40, -40, -30
  }),
  square_weights({ // queen
    -20, -10, -10, -5, -5, -10, -10, -20,
    -10, 0, 0, 0, 0, 0, 0, -10,
    -10, 5, 5, 5, 5, 5, 0, -10,
    0, 0, 5, 5, 5, 5, 0, -5,
    -5, 0, 5, 5, 5, 5, 0, -5,
    -10, 0, 5, 5, 5, 5, 0, -10,
    -10, 0, 0, 0, 0, 0, 0, -10,
    -20, -10, -10, -5, -5, -10, -10, -20
  }),
  square_weights({ // rook
     0, 0, 0, 5, 5, 0, 0, 0,
    -5, 0, 0, 0, 0, 0, 0, -5,
    -5, 0, 0, 0, 0, 0, 0, -5,
    -5, 0, 0, 0, 0, 0, 0, -5,
    -5, 0, 0, 0, 0, 0, 0, -5,
    -5, 0, 0, 0, 0, 0, 0, -5,
    5, 10, 10, 10, 10, 10, 10, 5,
    0, 0, 0, 0, 0, 0, 0, 0
  }),
  square_weights({ // bishop
    -20, -10, -10, -10, -10, -10, -10, -20,
    -10, 5, 0, 0, 0, 0, 5, -10,
    -10, 10, 10, 10, 10, 10, 10, -10,
    -10, 0, 10, 10, 10, 10, 0, -10,
    -10, 5, 5, 10, 10, 5, 5, -10,
    -10, 0, 5, 10, 10, 5, 0, -10,
    -10, 0, 0, 0, 0, 0, 0, -10,
    -20, -10, -10, -10, -10, -10, -10, -20
  }),
  square_weights({ // knight
    -50, -40, -30, -30, -30, -30, -40, -50,
    -40, -20, 0, 5, 5, 0, -20, -40,
    -30, 5, 10, 15, 15, 10, 5, -30,
    -30, 0, 15, 20, 20, 15, 0, -30,
    -30, 5, 15, 20, 20, 15, 5, -30,
    -30, 0, 10, 15, 15, 10, 0, -30,
    -40, -20, 0, 0, 0, 0, -20, -40,
    -50, -40, -30, -30, -30, -30, -40, -50
  }),
  square_weights({ // pawn
    0, 0, 0, 0, 0, 0, 0, 0,
    5, 10, 10, -20, -20, 10, 10, 5,
    5, -5, -10, 0, 0, -10, -5, 5,
    0, 0, 0, 20, 20, 0, 0, 0,
    5, 5, 10, 25, 25, 10, 5, 5,
    10, 10, 20, 30, 30, 20, 10, 10,
    50, 50, 50, 50, 50, 50, 50, 50,
    0, 0, 0, 0, 0, 0, 0, 0
  })
};

static_assert(LS_ARRAYSIZE(SquareWeights) == _chess_piece_type_count);

constexpr int64_t PieceScores[] = { 0, 100000, 950, 563, 333, 305, 100 }; // Chess piece values from `https://en.wikipedia.org/wiki/Chess_piece_relative_value#Alternative_valuations > AlphaZero`.

int64_t evaluate_chess_board(const chess_board &board)
{
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
    list<chess_move> moves;
    list_clear(&moves);
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
struct moves_with_score
{
  chess_move moves[MaxDepth];
  int64_t score;

  moves_with_score() = default;
  moves_with_score(const chess_move moves[MaxDepth], const int64_t score) : score(score)
  {
    lsMemcpy(this->moves, moves, LS_ARRAYSIZE(this->moves));
  }
};

constexpr bool UseCache = false;

template <size_t MaxDepth>
struct alpha_beta_minimax_cache
{
  static constexpr size_t MaxQuiescenceDepth = 20;

  list<chess_move> movesAtLevel[MaxDepth];
  chess_move currentMove[MaxDepth + MaxQuiescenceDepth];
#ifdef _DEBUG
  size_t nodesVisited = 0;
  size_t quiescenceNodesVisited = 0;
  size_t duplicatesRejected = 0;
  chess_move highestMove[MaxDepth];
  chess_move lowestMove[MaxDepth];
  int64_t lowestScore = lsMaxValue<int64_t>();
  int64_t highestScore = lsMinValue<int64_t>();

  int64_t stepMin[MaxDepth];
  int64_t stepMax[MaxDepth];
#endif

  constexpr static size_t hashBits = 20;
  constexpr static size_t hashValues = (1ULL << hashBits);
  constexpr static size_t hashMask = hashValues - 1;

  chess_hash_board *pCache = nullptr;

  piece_move_map<true> pieceMovesWithNonCapture;
  piece_move_map<false> pieceMoves[2];
  list<chess_move> quiescenceMovesAtLevel[MaxQuiescenceDepth];

  int64_t ticksPerLayer[MaxDepth + 1] = {};

  alpha_beta_minimax_cache()
  {
#ifdef _DEBUG
    for (size_t i = 0; i < MaxDepth; i++)
    {
      stepMin[i] = lsMaxValue<int64_t>();
      stepMax[i] = lsMinValue<int64_t>();
    }
#endif
  }

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
  board.score = (int32_t)ret.score;
  board.move = ret.move;
  cache.pCache[hash] = std::move(board);
}

template <bool FindMin, size_t CacheDepth, size_t MaxDepth = alpha_beta_minimax_cache<CacheDepth>::MaxQuiescenceDepth>
int64_t quiescence_alpha_beta_step(const chess_board &board, int64_t alpha, int64_t beta, alpha_beta_minimax_cache<CacheDepth> &cache, const size_t depthIndex = 0)
{
  if (board.hasBlackWon)
    return -PieceScores[cpT_king];
  else if (board.hasWhiteWon)
    return PieceScores[cpT_king];
  else if (depthIndex == MaxDepth)
    return evaluate_chess_board(board);

  list<chess_move> &moves = cache.quiescenceMovesAtLevel[depthIndex];
  LS_DEBUG_ERROR_ASSERT(get_valid_quiescence_moves(moves, board, cache.pieceMoves[0], cache.pieceMoves[1]));

  if (!moves.count)
    return evaluate_chess_board(board);

  int64_t score = FindMin ? lsMaxValue<int64_t>() : lsMinValue<int64_t>();

  for (const chess_move move : moves)
  {
#ifdef _DEBUG
    cache.quiescenceNodesVisited++;
#endif

    const chess_board after = perform_move(board, move);
    cache.currentMove[CacheDepth + depthIndex] = move;

    const int64_t moveScore = quiescence_alpha_beta_step<!FindMin, CacheDepth, MaxDepth>(after, alpha, beta, cache, depthIndex + 1);

    if constexpr (FindMin)
    {
      if (moveScore < score)
      {
        score = moveScore;

        if (score < beta)
          beta = score;

        if (score <= alpha)
          break;
      }
    }
    else
    {
      if (moveScore > score)
      {
        score = moveScore;

        if (score > alpha)
          alpha = score;

        if (score >= beta)
          break;
      }
    }
  }

  return score;
}

constexpr bool UseQuiescenceSearch = true;

template <bool FindMin, size_t MaxDepth, size_t DepthIndex = 0>
moves_with_score<MaxDepth> alpha_beta_step(const chess_board &board, int64_t alpha, int64_t beta, alpha_beta_minimax_cache<MaxDepth> &cache)
{
  static_assert(DepthIndex <= MaxDepth);

  if constexpr (FindMin)
    if (board.hasWhiteWon)
      return moves_with_score<MaxDepth>(cache.currentMove, PieceScores[cpT_king]);

  if constexpr (!FindMin)
    if (board.hasBlackWon)
      return moves_with_score<MaxDepth>(cache.currentMove, -PieceScores[cpT_king]);

  if constexpr (DepthIndex == MaxDepth)
  {
    int64_t score;
    const int64_t begin = __rdtsc();

    if constexpr (UseQuiescenceSearch)
      score = quiescence_alpha_beta_step<!FindMin>(board, alpha, beta, cache);
    else
      score = evaluate_chess_board(board);

    const moves_with_score<MaxDepth> ret = moves_with_score<MaxDepth>(cache.currentMove, score);

#ifdef _DEBUG
    if (ret.score > cache.highestScore)
    {
      lsMemcpy(cache.highestMove, cache.currentMove, MaxDepth);
      cache.highestScore = ret.score;
    }

    if (ret.score < cache.lowestScore)
    {
      lsMemcpy(cache.lowestMove, cache.currentMove, MaxDepth);
      cache.lowestScore = ret.score;
    }
#endif

    const int64_t end = __rdtsc();
    cache.ticksPerLayer[DepthIndex] += end - begin;

    return ret;
  }
  else
  {
    const int64_t begin = __rdtsc();

    list<chess_move> &moves = cache.movesAtLevel[DepthIndex];
    LS_DEBUG_ERROR_ASSERT(get_all_valid_ordered_moves(moves, board, cache.pieceMoves[0], cache.pieceMovesWithNonCapture));
    moves_with_score<MaxDepth> ret;
    ret.score = FindMin ? lsMaxValue<int64_t>() : lsMinValue<int64_t>();

    for (const chess_move move : moves)
    {
#ifdef _DEBUG
      cache.nodesVisited++;
#endif

      const chess_board after = perform_move(board, move);
      cache.currentMove[DepthIndex] = move;

      const moves_with_score<MaxDepth> moveRating = alpha_beta_step<!FindMin, MaxDepth, DepthIndex + 1>(after, alpha, beta, cache);

#ifdef _DEBUG
      cache.stepMin[DepthIndex] = lsMin(moveRating.score, cache.stepMin[DepthIndex]);
      cache.stepMax[DepthIndex] = lsMax(moveRating.score, cache.stepMax[DepthIndex]);
#endif

      if constexpr (FindMin)
      {
        if (moveRating.score < ret.score)
        {
          ret = moveRating;

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
          ret = moveRating;

          if (ret.score > alpha)
            alpha = ret.score;

          if (ret.score >= beta)
            break;
        }
      }
    }

    const int64_t end = __rdtsc();
    cache.ticksPerLayer[DepthIndex] += end - begin;

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

template <bool IsWhite>
chess_move get_alpha_beta_move(const chess_board &board)
{
  constexpr size_t Depth = 6;

#ifdef _DEBUG
  const int64_t before = lsGetCurrentTimeNs();
#endif

  alpha_beta_minimax_cache<Depth> cache;
  LS_DEBUG_ERROR_ASSERT(alpha_beta_minimax_cache_create(cache));

  const moves_with_score<Depth> moveInfo = alpha_beta_step<!IsWhite>(board, lsMinValue<int64_t>(), lsMaxValue<int64_t>(), cache);

#ifdef _DEBUG
  const int64_t after = lsGetCurrentTimeNs();

  print(FU(Group)(cache.nodesVisited), " + ", FU(Group)(cache.quiescenceNodesVisited), " nodes visited (in ", FF(Max(5))((after - before) * 1e-9f), "s, ", FF(Max(9), Group)((cache.nodesVisited + cache.quiescenceNodesVisited) / ((after - before) * 1e-9f)), "/s)\n");

  print("\nBest Moves (rating: ", moveInfo.score, "):\n");

  for (size_t i = 0; i < Depth; i++)
  {
    print_move(moveInfo.moves[i]);
    print(", ");
  }

  print("\nBest move combination for white (rating: ", cache.highestScore, "):\n");

  for (size_t i = 0; i < Depth; i++)
  {
    print_move(cache.highestMove[i]);
    print(", ");
  }

  print("\nBest move combination for black (rating: ", cache.lowestScore, "):\n");

  for (size_t i = 0; i < Depth; i++)
  {
    print_move(cache.lowestMove[i]);
    print(", ");
  }

  print("\nRating Distribution:\n");

  for (size_t i = 0; i < Depth; i++)
    print(cache.stepMin[i], " ~ ", cache.stepMax[i], ", ");

  print('\n');
#endif

  print("\nTotal ticks: 100% (", FI(Group)(cache.ticksPerLayer[0]), ")\n");

  for (size_t i = 0; i < LS_ARRAYSIZE(cache.ticksPerLayer) - 1; i++)
  {
    const int64_t ticks = cache.ticksPerLayer[i] - cache.ticksPerLayer[i + 1];
    print("Layer ", i, ": ", FF(Min(8), Max(8))((ticks * 100.f) / cache.ticksPerLayer[0]), "% (", FI(Group)(ticks), ")\n");
  }

  print("Layer ", Depth, ": ", FF(Min(8), Max(8))((cache.ticksPerLayer[Depth] * 100.f) / cache.ticksPerLayer[0]), "% (", FI(Group)(cache.ticksPerLayer[Depth]), ")\n");

  print('\n');

  return moveInfo.moves[0];
}

chess_move get_alpha_beta_move_white(const chess_board &board)
{
  return get_alpha_beta_move<true>(board);
}

chess_move get_alpha_beta_move_black(const chess_board &board)
{
  return get_alpha_beta_move<false>(board);
}

//////////////////////////////////////////////////////////////////////////

template <size_t Depth, size_t MaxDepth, bool FindMin>
moves_with_score<MaxDepth> alpha_beta_aspiration(const chess_board &board, const int64_t guess, alpha_beta_minimax_cache<MaxDepth> &cache)
{
  constexpr int64_t delta = 50;
  const int64_t alpha = guess - delta;
  const int64_t beta = guess + delta;

  moves_with_score<MaxDepth> ret = alpha_beta_step<FindMin, MaxDepth, MaxDepth - Depth>(board, alpha, beta, cache);

  print("\taspiration: ", Depth, " / ", MaxDepth, ": ", ret.score, " (", alpha, " ~ ", beta, ")");

  if (ret.score <= alpha)
    ret = alpha_beta_step<FindMin, MaxDepth, MaxDepth - Depth>(board, lsMinValue<int64_t>(), beta, cache);
  else if (ret.score >= beta)
    ret = alpha_beta_step<FindMin, MaxDepth, MaxDepth - Depth>(board, alpha, lsMaxValue<int64_t>(), cache);

  print(" => ", ret.score, '\n');

  return ret;
}

template <bool FindMin, size_t MaxDepth, size_t Depth = 1>
void alpha_beta_iterative_deepen(const chess_board &board, alpha_beta_minimax_cache<MaxDepth> &cache, moves_with_score<MaxDepth> &ret)
{
  static_assert(Depth > 0);

  if constexpr (Depth == 1)
  {
    ret = alpha_beta_step<FindMin, MaxDepth, MaxDepth - Depth>(board, lsMinValue<int64_t>(), lsMaxValue<int64_t>(), cache);

    print("\titerative deepen: ", Depth, " / ", MaxDepth, ": ", ret.score, '\n');

    if constexpr (MaxDepth > Depth)
      alpha_beta_iterative_deepen<FindMin, MaxDepth, Depth + 1>(board, cache, ret);
  }
  else
  {
    if (lsAbs(ret.score) >= PieceScores[cpT_king])
    {
      lsMemmove(ret.moves, ret.moves + MaxDepth - (Depth - 1), 1);
      return;
    }

    ret = alpha_beta_aspiration<Depth, MaxDepth, FindMin>(board, ret.score, cache);

    if constexpr (MaxDepth > Depth)
      alpha_beta_iterative_deepen<FindMin, MaxDepth, Depth + 1>(board, cache, ret);
  }
}

template <bool IsWhite>
chess_move get_complex_move(const chess_board &board)
{
  constexpr size_t Depth = 6;

#ifdef _DEBUG
  const int64_t before = lsGetCurrentTimeNs();
#endif

  alpha_beta_minimax_cache<Depth> cache;
  LS_DEBUG_ERROR_ASSERT(alpha_beta_minimax_cache_create(cache));

  moves_with_score<Depth> moveInfo;
  alpha_beta_iterative_deepen<!IsWhite>(board, cache, moveInfo);

#ifdef _DEBUG
  const int64_t after = lsGetCurrentTimeNs();

  print(FU(Group)(cache.nodesVisited), " + ", FU(Group)(cache.quiescenceNodesVisited), " nodes visited (in ", FF(Max(5))((after - before) * 1e-9f), "s, ", FF(Max(9), Group)((cache.nodesVisited + cache.quiescenceNodesVisited) / ((after - before) * 1e-9f)), "/s)\n");

  print("\nBest Moves (rating: ", moveInfo.score, "):\n");

  for (size_t i = 0; i < Depth; i++)
  {
    print_move(moveInfo.moves[i]);
    print(", ");
  }

  print("\nBest move combination for white (rating: ", cache.highestScore, "):\n");

  for (size_t i = 0; i < Depth; i++)
  {
    print_move(cache.highestMove[i]);
    print(", ");
  }

  print("\nBest move combination for black (rating: ", cache.lowestScore, "):\n");

  for (size_t i = 0; i < Depth; i++)
  {
    print_move(cache.lowestMove[i]);
    print(", ");
  }

  print("\nRating Distribution:\n");

  for (size_t i = 0; i < Depth; i++)
    print(cache.stepMin[i], " ~ ", cache.stepMax[i], ", ");

  print('\n');
#endif

  return moveInfo.moves[0];
}

chess_move get_complex_move_white(const chess_board &board)
{
  return get_complex_move<true>(board);
}

chess_move get_complex_move_black(const chess_board &board)
{
  return get_complex_move<false>(board);
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
      lsSetConsoleColor(piece.isWhite ? lsCC_White : lsCC_Black, ((x ^ y) & 1) ? lsCC_BrightCyan : lsCC_BrightBlue);
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

bool replay_move_sequence(const chess_board startPosition, const std::initializer_list<chess_move> &moves)
{
  bool isBotTurn = true;
  chess_board board = startPosition;

  for (const chess_move move : moves)
  {
    if (isBotTurn)
    {
      chess_move m;
      if (board.isWhitesTurn)
        m = get_alpha_beta_move_white(board);
      else
        m = get_alpha_beta_move_black(board);

      if (m != move)
        return false;
    }

    board = perform_move(board, move);
    isBotTurn = !isBotTurn;
  }

  return true;
}

DEFINE_TESTABLE(midgame_puzzle_test)
{
  lsResult result = lsR_Success;

  print("Test Puzzle #1\n");

  TESTABLE_ASSERT_TRUE(replay_move_sequence(get_board_from_starting_position("r..q.b.r\n..p.kpp.\nppQp.n..\n...PP.p.\n........\n........\nPPP...PP\nRN...RK."),
    {
      chess_move(vec2i8(4, 4), vec2i8(5, 5), cmt_pawn_capture),
      chess_move(vec2i8(6, 6), vec2i8(5, 5), cmt_pawn_capture),
      chess_move(vec2i8(5, 0), vec2i8(4, 0), cmt_rook),
    }));;

  print("Test Puzzle #2\n");

  TESTABLE_ASSERT_TRUE(replay_move_sequence(get_board_from_starting_position("r.q..r..\npbb..p..\n.p...knQ\n..p..p..\n........\n..PP....\nPP....PP\nRNB.R.K."),
    {
      chess_move(vec2i8(2, 0), vec2i8(6, 4), cmt_bishop)
    }));;

  print("Test Puzzle #3\n");

  TESTABLE_ASSERT_TRUE(replay_move_sequence(get_board_from_starting_position("r...kb.r\nppp.pppp\nn....n..\n....Q...\n.....B..\n..N.KP..\nPPP...qP\n...R..NR"),
    {
      chess_move(vec2i8(4, 4), vec2i8(1, 4), cmt_queen_straight)
    }));;

epilogue:
  return result;
}

DEFINE_TESTABLE(fen_parsing_test)
{
  lsResult result = lsR_Success;
  
  const chess_board s = get_board_from_starting_position("r...kb.r\nppp.pppp\nn....n..\n....Q...\n.....B..\n..N.KP..\nPPP...qP\n...R..NR");
  const chess_board f = get_board_from_fen("r3kb1r/ppp1pppp/n4n2/4Q3/5B2/2N1KP2/PPP3qP/3R2NR w");

  print_board(s);
  print_board(f);

  for (int8_t y = 0; y < BoardWidth; y++)
  {
    for (int8_t x = 0; x < BoardWidth; x++)
    {
      const vec2i8 pos = vec2i8(x, y);
      TESTABLE_ASSERT_EQUAL(s[pos], f[pos]);
    }
  }

  TESTABLE_ASSERT_EQUAL((bool)f.isWhitesTurn, true);

epilogue:
  return result;
}
