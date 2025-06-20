#pragma once

#include "core.h"
#include "small_list.h"

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
  chess_piece(chess_piece &&move) noexcept : piece(move.piece), isWhite(move.isWhite), hasMoved(move.hasMoved), lastWasDoubleStep(lastWasDoubleStep) { move.piece = cpT_none; }

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

struct chess_move
{
  uint8_t startX : 3;
  uint8_t startY : 3;
  uint8_t targetX : 3;
  uint8_t targetY : 3;

  uint8_t isPromotion : 1 = false;
  uint8_t isPromotedToQueen : 1 = false; // other option is knight. Needed so we don't have to include all other pieces because of weird alligment issues.

  chess_move() = default;
  chess_move(const vec2i8 start, const vec2i8 target) : startX(start.x), startY(start.y), targetX(target.x), targetY(target.y), isPromotion(false), isPromotedToQueen(false)
  {
    lsAssert(start.x >= 0 && start.x < BoardWidth && start.y >= 0 && start.y < BoardWidth);
    lsAssert(target.x >= 0 && target.x < BoardWidth && target.y >= 0 && target.y < BoardWidth);
  }
};

static_assert(sizeof(chess_move) == sizeof(uint16_t));

lsResult get_all_valid_moves(const chess_board &board, small_list<chess_move> &moves);
chess_board perform_move(const chess_board &board, const chess_move move);

