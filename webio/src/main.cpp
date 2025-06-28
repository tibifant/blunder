#include <stdio.h>
#include <exception>
#include <stdlib.h>
#include <malloc.h>

#define ASIO_STANDALONE 1
#define ASIO_NO_EXCEPTIONS 1

#define BLUNDER_WEBIO_LOCALHOST
#define BLUNDER_WEBIO_HOSTNAME "https://hostname_not_configured"

namespace asio
{
  namespace detail
  {
    template <typename Exception>
    void throw_exception(const Exception &e)
    {
#ifdef _MSC_VER
      __debugbreak(); // windows only, sorry!
#endif
      printf("Exception thrown: %s.\n", e.what());
    }
  }
}

//////////////////////////////////////////////////////////////////////////

#ifdef _MSC_VER
#pragma warning (push)
#pragma warning (disable: 4702) // unreachable (somewhere in json.h)
#endif

//////////////////////////////////////////////////////////////////////////

#ifdef _MSC_VER
#pragma warning (push, 0)
#else
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wtype-limits"
#endif
#include "crow.h"
#include "crow/middlewares/cors.h"
#ifdef _MSC_VER
#pragma warning (pop)
#else
#pragma GCC diagnostic pop
#endif

//////////////////////////////////////////////////////////////////////////

#include "core.h"
#include "blunder.h"

//////////////////////////////////////////////////////////////////////////

crow::response handle_get_board(const crow::request &req);
crow::response handle_get_valid_moves(const crow::request &req);
crow::response handle_move(const crow::request &req);
crow::response handle_restart(const crow::request &req);

//////////////////////////////////////////////////////////////////////////

static chess_board _CurrentBoard = chess_board::get_starting_point();

//////////////////////////////////////////////////////////////////////////

int32_t main(const int32_t argc, const char **pArgv)
{
  // Ignore Args for now.
  (void)argc;
  (void)pArgv;

  sformatState_ResetCulture();
  cpu_info::DetectCpuFeatures();

  if (!cpu_info::avx2Supported || !cpu_info::aesNiSupported)
  {
    print_error_line("CPU '", cpu_info::GetCpuName(), "' is not supported (AVX2, AES/NI required)");
    return EXIT_FAILURE;
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

  print("Blunder WebIO (built " __DATE__ " " __TIME__ ") running on ", cpu_info::GetCpuName(), ".\n");

  {
    crow::App<crow::CORSHandler> app;

    auto &cors = app.get_middleware<crow::CORSHandler>();
#ifndef BLUNDER_WEBIO_LOCALHOST
    cors.global().origin(BLUNDER_WEBIO_HOSTNAME);
#else
    cors.global().origin("*");
#endif

    CROW_ROUTE(app, "/get_board").methods(crow::HTTPMethod::POST)([](const crow::request &req) { return handle_get_board(req); });
    CROW_ROUTE(app, "/get_valid_moves").methods(crow::HTTPMethod::POST)([](const crow::request &req) { return handle_get_valid_moves(req); });
    CROW_ROUTE(app, "/move").methods(crow::HTTPMethod::POST)([](const crow::request &req) { return handle_move(req); });
    CROW_ROUTE(app, "/restart").methods(crow::HTTPMethod::POST)([](const crow::request &req) { return handle_restart(req); });

    app.port(21110).multithreaded().run();
  }

  return EXIT_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////

crow::response handle_get_board(const crow::request &req)
{
  (void)req;

  crow::json::wvalue ret;

  ret["isWhitesTurn"] = _CurrentBoard.isWhitesTurn;
  ret["hasBlackWon"] = _CurrentBoard.hasBlackWon;
  ret["hasWhiteWon"] = _CurrentBoard.hasWhiteWon;

  const char pieceChars[] = " KQRBNP";

  for (int8_t y = 0; y < BoardWidth; y++)
  {
    for (int8_t x = 0; x < BoardWidth; x++)
    {
      const chess_piece piece = _CurrentBoard[vec2i8(x, y)];
      if (piece.piece != cpT_none)
      {
        ret[x][y]["piece"] = sformat(pieceChars[piece.piece]);
        ret[x][y]["isWhite"] = piece.isWhite;
      }
    }
  }

  return ret;
}

crow::response handle_get_valid_moves(const crow::request &req)
{
  (void)req;

  crow::json::wvalue ret;

  small_list<chess_move> moves;
  if (LS_FAILED(get_all_valid_moves(_CurrentBoard, moves)))
    return crow::response(crow::status::INTERNAL_SERVER_ERROR);

  uint16_t i = 0;

  for (const auto &move : moves)
  {
    ret[i]["originX"] = move.startX;
    ret[i]["originY"] = move.startY;
    ret[i]["destinationX"] = move.targetX;
    ret[i]["destinationY"] = move.targetY;
    ret[i]["isPromotion"] = move.isPromotion;

    if (move.isPromotion)
      ret[i]["isPromotionToQueen"] = move.isPromotedToQueen;

    i++;
  }

  return ret;
}

crow::response handle_move(const crow::request &req)
{
  auto body = crow::json::load(req.body);

  if (!body || !body.has("originX") || !body.has("originY") || !body.has("destinationX") || !body.has("destinationY") || !body.has("isPromotion"))
    return crow::response(crow::status::BAD_REQUEST);

  const int8_t originX = (int8_t)(body["originX"].i());
  const int8_t originY = (int8_t)(body["originY"].i());
  const int8_t destX = (int8_t)(body["destinationX"].i());
  const int8_t destY = (int8_t)(body["destinationY"].i());
  const bool isPromotion = body["isPromotion"].b();

  if (originX < 0 || originX >= BoardWidth || originY < 0 || originY >= BoardWidth || destX < 0 || destX >= BoardWidth || destY < 0 || destY >= BoardWidth)
    return crow::response(crow::status::BAD_REQUEST);

  small_list<chess_move> moves;
  if (LS_FAILED(get_all_valid_moves(_CurrentBoard, moves)))
    return crow::response(crow::status::INTERNAL_SERVER_ERROR);

  std::optional<chess_move> chosenMove;

  for (const auto &move : moves)
  {
    if (move.startX == originX && move.startY == originY && move.targetX == destX && move.targetY == destY && move.isPromotion == isPromotion)
    {
      chosenMove = move;
      break;
    }
  }

  if (!chosenMove.has_value())
    return crow::response(crow::status::BAD_REQUEST);

  if (chosenMove.value().isPromotion)
  {
    if (!body.has("isPromotionToQueen"))
      return crow::response(crow::status::BAD_REQUEST);

    chosenMove.value().isPromotedToQueen = body["isPromotionToQueen"].b();
  }

  // Perform move.
  _CurrentBoard = perform_move(_CurrentBoard, chosenMove.value());

  // AI move.
  {
    if (LS_FAILED(get_all_valid_moves(_CurrentBoard, moves)))
      return crow::response(crow::status::INTERNAL_SERVER_ERROR);

    const size_t moveIdx = lsGetRand() % moves.count;
    _CurrentBoard = perform_move(_CurrentBoard, moves[moveIdx]);
  }

  return crow::response(crow::status::OK);
}

crow::response handle_restart(const crow::request &req)
{
  auto body = crow::json::load(req.body);

  if (!body || !body.has("type") || body["type"].s() == "default")
    _CurrentBoard = chess_board::get_starting_point();
  else
    return crow::response(crow::status::NOT_FOUND);

  return crow::response(crow::status::OK);
}
