#pragma once
#pragma once

#include "core.h"
#include "list.h"

enum chess_piece_type : uint8_t
{
  cpT_none, // `none` has to always be 0 for easy checks.

  cpT_king,
  cpT_queen,
  cpT_rook,
  cpT_bishop,
  cpT_knight,
  cpT_pawn,

  _chess_piece_type_count
};

static_assert(_chess_piece_type_count < (1 << 4));

struct chess_piece
{
  chess_piece_type piece : 4 = cpT_none;
  uint8_t isWhite : 1 = false;
  uint8_t hasMoved : 1 = false;
  uint8_t lastWasDoubleStep : 1 = false; // this is needed to know if a pawn can be captured. Needs to be reset after every move!

  chess_piece() = default;
  chess_piece(const chess_piece_type type, const bool isWhite) : piece(type), isWhite(isWhite), lastWasDoubleStep(false), hasMoved(false) {}
  chess_piece(const chess_piece &) = default;

  chess_piece(chess_piece &&move) noexcept : piece(move.piece), isWhite(move.isWhite), hasMoved(move.hasMoved), lastWasDoubleStep(move.lastWasDoubleStep)
  {
    move.piece = cpT_none;
    move.lastWasDoubleStep = false;
  }

  chess_piece &operator = (const chess_piece &) = default;

  chess_piece &operator = (chess_piece &&move) noexcept
  {
    piece = move.piece;
    isWhite = move.isWhite;
    hasMoved = move.hasMoved;
    lastWasDoubleStep = move.lastWasDoubleStep;

    move.piece = cpT_none;
    return *this;
  }

  bool operator==(const chess_piece other) const
  {
    return piece == other.piece && isWhite == other.isWhite;
  }
};

static_assert(sizeof(chess_piece) == sizeof(uint8_t));

constexpr size_t BoardWidth = 8;

struct chess_board
{
  chess_piece board[BoardWidth * BoardWidth];
  uint8_t isWhitesTurn : 1 = true;
  uint8_t hasWhiteWon : 1 = false;
  uint8_t hasBlackWon : 1 = false;

  chess_piece &operator[](const vec2i8 pos)
  {
    lsAssert(pos.x >= 0 && pos.x < BoardWidth && pos.y >= 0 && pos.y < BoardWidth);
    return board[pos.y * BoardWidth + pos.x];
  }

  const chess_piece &operator[](const vec2i8 pos) const
  {
    lsAssert(pos.x >= 0 && pos.x < BoardWidth && pos.y >= 0 && pos.y < BoardWidth);
    return board[pos.y * BoardWidth + pos.x];
  }

  inline bool has_any(const chess_piece piece) const
  {
    for (size_t i = 0; i < LS_ARRAYSIZE(board); i++)
      if (board[i] == piece)
        return true;

    return false;
  }

  static chess_board get_starting_point();
};

enum chess_move_type : uint8_t
{
  cmt_invalid,
  cmt_pawn,
  cmt_pawn_double_step,
  cmt_pawn_en_passant,
  cmt_pawn_promotion,
  cmt_pawn_capture,
  cmt_knight,
  cmt_queen_straight,
  cmt_queen_diagonal,
  cmt_rook,
  cmt_bishop,
  cmt_king,
  cmt_king_castle,
};

struct chess_move
{
  uint8_t startX : 3;
  uint8_t startY : 3;
  uint8_t targetX : 3;
  uint8_t targetY : 3;

  uint8_t isPromotion : 1 = false;
  uint8_t isPromotedToQueen : 1 = false; // other option is knight. Needed so we don't have to include all other pieces because of weird alligment issues.

#ifdef _DEBUG
  chess_move_type moveType = cmt_invalid; // only for asserting!
#endif

  chess_move() = default;
  chess_move(const chess_move &) = default;
  chess_move(chess_move &&) = default;
  chess_move &operator =(const chess_move &) = default;
  chess_move &operator =(chess_move &&) = default;

  chess_move(const vec2i8 start, const vec2i8 target, const chess_move_type type) : startX(start.x), startY(start.y), targetX(target.x), targetY(target.y), isPromotion(false), isPromotedToQueen(false)
  {
#ifdef _DEBUG
    lsAssert(type != cmt_invalid);
    moveType = type;
#else
    (void)type;
#endif

    lsAssert(start.x >= 0 && start.x < BoardWidth && start.y >= 0 && start.y < BoardWidth);
    lsAssert(target.x >= 0 && target.x < BoardWidth && target.y >= 0 && target.y < BoardWidth);
  }
};

#ifndef _DEBUG
static_assert(sizeof(chess_move) == sizeof(uint16_t));
#endif

struct chess_hash_board
{
  uint8_t nibbleMap[8 * 4];
  uint32_t isWhitesTurn : 1;
  int32_t score : 31;
  chess_move move;

  inline bool operator==(const chess_hash_board &other) const
  {
    return isWhitesTurn == other.isWhitesTurn && (memcmp(nibbleMap, other.nibbleMap, sizeof(nibbleMap)) == 0);
  }
};

static_assert(_chess_piece_type_count <= (1 << 3));

#ifndef _DEBUG
static_assert(sizeof(chess_hash_board) == 8 * 8 / 2 + 4 + 4);
#endif

chess_hash_board chess_hash_board_create(const chess_board &board);
uint64_t lsHash(const chess_hash_board &board);

lsResult get_all_valid_moves(const chess_board &board, list<chess_move> &moves);

chess_board perform_move(const chess_board &board, const chess_move move);
int64_t evaluate_chess_board(const chess_board &board);

chess_move get_minimax_move_white(const chess_board &board);
chess_move get_minimax_move_black(const chess_board &board);
chess_move get_alpha_beta_move_white(const chess_board &board);
chess_move get_alpha_beta_move_black(const chess_board &board);
chess_move get_complex_move_white(const chess_board &board);
chess_move get_complex_move_black(const chess_board &board);

void print_board(const chess_board &board);
void print_move(const chess_move move);
