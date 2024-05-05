#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <cctype>
#include <algorithm>
#include <unordered_map>
#include <iomanip>
#include <nlohmann/json.hpp>
#include <fstream>
#include <random>


#define MATE_VALUE 16000
#define MAX_VALUE 16500

int REGULAR_DEPTH;
int MAX_DEPTH;

int pawn_list[] = {
    0,  0,  0,  0,  0,  0,  0,  0,
    50, 50, 50, 50, 50, 50, 50, 50,
    10, 10, 20, 30, 30, 20, 10, 10,
    5,  5, 10, 25, 25, 10,  5,  5,
    0,  0,  0, 20, 20,  0,  0,  0,
    5, -5,-10,  0,  0,-10, -5,  5,
    5, 10, 10,-20,-20, 10, 10,  5,
    0,  0,  0,  0,  0,  0,  0,  0
};

int knight_list[] = {
    -50,-40,-30,-30,-30,-30,-40,-50,
    -40,-20,  0,  0,  0,  0,-20,-40,
    -30,  0, 10, 15, 15, 10,  0,-30,
    -30,  5, 15, 20, 20, 15,  5,-30,
    -30,  0, 15, 20, 20, 15,  0,-30,
    -30,  5, 10, 15, 15, 10,  5,-30,
    -40,-20,  0,  5,  5,  0,-20,-40,
    -50,-40,-30,-30,-30,-30,-40,-50,
};

int bishop_list[] = {
    -20,-10,-10,-10,-10,-10,-10,-20,
    -10,  0,  0,  0,  0,  0,  0,-10,
    -10,  0,  5, 10, 10,  5,  0,-10,
    -10,  5,  5, 10, 10,  5,  5,-10,
    -10,  0, 10, 10, 10, 10,  0,-10,
    -10, 10, 10, 10, 10, 10, 10,-10,
    -10,  5,  0,  0,  0,  0,  5,-10,
    -20,-10,-10,-10,-10,-10,-10,-20,
};

int rook_list[] = {
      0,  0,  0,  0,  0,  0,  0,  0,
      5, 10, 10, 10, 10, 10, 10,  5,
     -5,  0,  0,  0,  0,  0,  0, -5,
     -5,  0,  0,  0,  0,  0,  0, -5,
     -5,  0,  0,  0,  0,  0,  0, -5,
     -5,  0,  0,  0,  0,  0,  0, -5,
     -5,  0,  0,  0,  0,  0,  0, -5,
      0,  0,  0,  5,  5,  0,  0,  0
};

int queen_opening_list[] = {
    -30,-40,-40,-50,-50,-40,-40,-30,
    -30,-40,-40,-50,-50,-40,-40,-30,
    -30,-40,-40,-50,-50,-40,-40,-30,
    -30,-40,-40,-50,-50,-40,-40,-30,
    -20,-30,-30,-40,-40,-30,-30,-20,
    -10,-20,-20,-20,-20,-20,-20,-10,
     20, 20,  0,  0,  0,  0, 20, 20,
     20, 30, 10,  0,  0, 10, 30, 20
};

int queen_list[] = {
    -20,-10,-10, -5, -5,-10,-10,-20,
    -10,  0,  0,  0,  0,  0,  0,-10,
    -10,  0,  5,  5,  5,  5,  0,-10,
     -5,  0,  5,  5,  5,  5,  0, -5,
      0,  0,  5,  5,  5,  5,  0, -5,
    -10,  5,  5,  5,  5,  5,  0,-10,
    -10,  0,  5,  0,  0,  0,  0,-10,
    -20,-10,-10, -5, -5,-10,-10,-20
};

int king_middle_list[] = {
    -30,-40,-40,-50,-50,-40,-40,-30,
    -30,-40,-40,-50,-50,-40,-40,-30,
    -30,-40,-40,-50,-50,-40,-40,-30,
    -30,-40,-40,-50,-50,-40,-40,-30,
    -20,-30,-30,-40,-40,-30,-30,-20,
    -10,-20,-20,-20,-20,-20,-20,-10,
     20, 20,  0,  0,  0,  0, 20, 20,
     20, 30, 10,  0,  0, 10, 30, 20
};

int king_end_list[] = {
    -50,-40,-30,-20,-20,-30,-40,-50,
    -30,-20,-10,  0,  0,-10,-20,-30,
    -30,-10, 20, 30, 30, 20,-10,-30,
    -30,-10, 30, 40, 40, 30,-10,-30,
    -30,-10, 30, 40, 40, 30,-10,-30,
    -30,-10, 20, 30, 30, 20,-10,-30,
    -30,-30,  0,  0,  0,  0,-30,-30,
    -50,-30,-30,-30,-30,-30,-30,-50
};

std::hash<std::string> hasher;
int trees_searched = 0;

std::vector<std::string> openings_raw;

struct ChessBoard {
    char pieces[64];
    char turn;
    bool whiteCanCastleKingSide;
    bool whiteCanCastleQueenSide;
    bool blackCanCastleKingSide;
    bool blackCanCastleQueenSide;
    bool blackCastled;
    bool whiteCastled;
    int enPassantSquare;
    int previousCapture;
    int turnNumber;
};

struct Move {
    int sq1;
    int sq2;
    int capture;
    int contest;
    int piece_class;
    int score;
};

Move killer_moves[20][2];

void updateKillerMove(int depth, Move move) {
    if (killer_moves[depth][0].sq1 != move.sq1 || killer_moves[depth][0].sq2 != move.sq2)
    {
        killer_moves[depth][1] = killer_moves[depth][0];
        killer_moves[depth][0] = move;
    }
}


struct MoveEval {
    Move move;
    int eval;
};

struct TranspositionEntry {
    int score;
    int depth;
    Move move;
};


std::string getSquareNotation(int square) 
{
    if (square < 0 || square > 63) {
        return "Invalid square";
    }

    char file = 'A' + (square % 8);
    char rank = '8' - (square / 8);

    std::string notation;
    notation += file;
    notation += rank;

    return notation;
}


int getSquareIndex(const std::string& squareNotation) {
    if (squareNotation.length() != 2) {
        return -1; // Invalid square notation
    }

    char file = std::toupper(squareNotation[0]);
    char rank = squareNotation[1];

    if (file < 'A' || file > 'H' || rank < '1' || rank > '8') {
        return -1; // Invalid square notation
    }

    int fileIndex = file - 'A';
    int rankIndex = '8' - rank;

    return rankIndex * 8 + fileIndex;
}


template<typename T>
T getRandomValue(const std::vector<T>& vec) {
    if (vec.empty()) {
        throw std::out_of_range("Vector is empty");
    }

    // Seed the random number generator
    std::random_device rd;
    std::mt19937 gen(rd());

    // Generate a random index within the range of the vector size
    std::uniform_int_distribution<> dis(0, vec.size() - 1);
    int randomIndex = dis(gen);

    // Return the randomly selected value
    return vec[randomIndex];
}

std::string sha256(const std::string& str) {
    // SHA-256 produces a 256-bit (32 bytes) hash
    constexpr size_t digest_size = 64;
    unsigned char hash[digest_size];

    // Your implementation of SHA-256 hashing algorithm here

    std::stringstream ss;
    for (size_t i = 0; i < digest_size; ++i) {
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
    }
    return ss.str().substr(0, 64); // Take first 64 characters
}

std::unordered_map<std::string, TranspositionEntry> transpositionTable;
//std::unordered_map<size_t, TranspositionEntry> transpositionTable;

//std::unordered_map<size_t, std::vector<Move>> opening_table;
std::unordered_map<std::string, std::vector<Move>> opening_table;

std::unordered_map<std::string, int> history_table;

std::string moveToString(Move move)
{
    std::string string = std::to_string(move.sq1) + std::to_string(move.sq2);
    return string;
}

void insertToHistory(Move move, char color, int depth)
{
    history_table[std::to_string(color) + moveToString(move)] += depth*depth;
}

//std::string generateHashKey(const ChessBoard* board) 
std::string generateHashKey(const ChessBoard* board) 
{
    std::string key = "";

    // Concatenate piece positions
    for (int i = 0; i < 64; ++i) {
        key += board->pieces[i];
    }

    // Concatenate turn
    key += board->turn;

    // Concatenate castle rights
    key += board->whiteCanCastleKingSide ? '1' : '0';
    key += board->whiteCanCastleQueenSide ? '1' : '0';
    key += board->blackCanCastleKingSide ? '1' : '0';
    key += board->blackCanCastleQueenSide ? '1' : '0';

    // Concatenate en passant square
    key += std::to_string(board->enPassantSquare);

    //return sha256(key);
    return key;
}

std::string generateHashKeySimple(const ChessBoard* board)
{
    std::string key = "";

    // Concatenate piece positions
    for (int i = 0; i < 64; ++i) {
        key += board->pieces[i];
    }

    // Concatenate turn
    key += board->turn;

    //return sha256(key);
    return key;
}


Move createMove(int sq1, int sq2, int capture = 0, int contest = 0, int piece_class = 0)
{
    Move move;
    move.sq1 = sq1;
    move.sq2 = sq2;
    move.capture = capture;
    move.contest = contest;
    move.piece_class = piece_class;

    return move;
}

MoveEval createMoveEval(Move move, int eval)
{
    MoveEval moveEval;
    moveEval.move = move;
    moveEval.eval = eval;

    return moveEval;
}


ChessBoard fenToChessBoard(std::string fen)
{
    ChessBoard board;

    std::memset(board.pieces, '-', 64);

    int index = 0;
    int rank = 0;
    for (char c : fen)
    {
        if (c == ' ')
        {
            break;
        }
        else if (c == '/')
        {
            rank++;
            index = rank * 8;
        }
        else if (std::isdigit(c))
        {
            index += c - '0';
        }
        else if (c == 'r' || c == 'n' || c == 'b' || c == 'q' || c == 'k' ||
            c == 'p' || c == 'R' || c == 'N' || c == 'B' || c == 'Q' ||
            c == 'K' || c == 'P')
        {
            board.pieces[index++] = c;
        }
    }

    size_t pos = fen.find(' ');
    std::string info = fen.substr(pos + 1);
    std::istringstream iss(info);
    std::string token;
    iss >> board.turn;
    iss >> token; // Castling rights
    board.whiteCanCastleKingSide = token.find('K') != std::string::npos;
    board.whiteCanCastleQueenSide = token.find('Q') != std::string::npos;
    board.blackCanCastleKingSide = token.find('k') != std::string::npos;
    board.blackCanCastleQueenSide = token.find('q') != std::string::npos;
    iss >> token; // En passant square
    if (token != "-") {
        int file = token[0] - 'a';
        int rank = 8 - (token[1] - '0');
        board.enPassantSquare = rank * 8 + file;
    }
    else {
        board.enPassantSquare = -1; // No en passant square
    }
    iss >> token;
    iss >> token;
    board.turnNumber = stoi(token);
    //std::cout << board.turnNumber;

    board.previousCapture = -1;

    return board;
}


void printBoard(ChessBoard* board)
{
    for (int i = 0; i < 64; i++)
    {
        std::cout << board->pieces[i] << " ";
        if ((i + 1) % 8 == 0)
        {
            std::cout << std::endl;
        }
    }
    return;
}


bool isInArray(int value, const std::vector<int>& arr) {
    auto it = std::find(arr.begin(), arr.end(), value);
    return it != arr.end();
}


bool isValidSquare(int square)
{
    if (square < 64 && square >= 0)
        return true;
    else
        return false;
}


bool isPieceThere(ChessBoard* board, int square)
{
    if (board->pieces[square] == '-')
        return false;
    else
        return true;
}


bool isPieceThereColor(ChessBoard* board, int square, char color)
{
    if (board->pieces[square] == '-')
        return false;
    else
    {
        if ((isupper(board->pieces[square]) && color == 'w') || (islower(board->pieces[square]) && color == 'b'))
            return true;
        else
            return false;
    }
}


char swapColor(char color)
{
    if (color == 'w')
        return 'b';
    else
        return 'w';
}


bool isPieceOfColor(char piece, char color)
{
    if (piece != '-')
    {
        if (color == 'w' && isupper(piece))
            return true;
        else if (color == 'b' && islower(piece))
            return true;
        else
            return false;
    }
    else
        return false;
}


int getRow(int square)
{
    return square / 8;
}


int getCol(int square)
{
    return square % 8;
}

int pieceToValue(char piece)
{
    if (toupper(piece) == 'P')
        return 100;
    else if (toupper(piece) == 'N')
        return 320;
    else if (toupper(piece) == 'B')
        return 340;
    else if (toupper(piece) == 'R')
        return 500;
    else if (toupper(piece) == 'Q')
        return 900;

    return 0;
}


std::vector<Move> getPawnMoves(ChessBoard* board, int square, char color)
{
    std::vector<Move> moves;

    int dir;
    if (color == 'w')
        dir = -1;
    else
        dir = 1;

    int row = getRow(square);

    int forward_square = square + 8 * dir;
    int double_forward_square = square + 16 * dir;

    if (isValidSquare(forward_square) && !isPieceThere(board, forward_square))
    {
        moves.push_back(createMove(square, forward_square, 0, 0, 0));

        if ((color == 'w' && row == 6) || (color == 'b' && row == 1))
        {
            if (isValidSquare(double_forward_square) && !isPieceThere(board, double_forward_square))
            {
                moves.push_back(createMove(square, double_forward_square, 0, 0, 0));
            }
        }
    }

    int diagonal_squares[] = { square - 1 + 8 * dir, square + 1 + 8 * dir };
    int dsquare;

    for (int i = 0; i < 2; i++)
    {
        dsquare = diagonal_squares[i];
        if (isValidSquare(dsquare) && (getRow(dsquare) == getRow(square) + dir))
        {
            if (isPieceThereColor(board, dsquare, swapColor(color)) || board->enPassantSquare == dsquare)
            {
                moves.push_back(createMove(square, dsquare, pieceToValue(board->pieces[dsquare]) - 10, 0, 0));
            }
        }
    }


    return moves;
}


std::vector<Move> getKnightMoves(ChessBoard* board, int square, char color)
{
    std::vector<Move> moves;

    int offsets[] = { -17, -15, -10, -6, 6, 10, 15, 17 };
    int new_square;
    int capture_points = 0;

    for (int offset : offsets)
    {
        new_square = square + offset;

        if (isValidSquare(new_square) && !isPieceThereColor(board, new_square, color))
        {
            if (abs(getCol(square) - getCol(new_square)) <= 2)
            {
                if (isPieceThereColor(board, new_square, swapColor(color)))
                    capture_points = pieceToValue(board->pieces[new_square]) - 32;
                else
                    capture_points = 0;
                
                moves.push_back(createMove(square, new_square, capture_points, 0, 1));
            }
        }
    }

    return moves;
}


std::vector<Move> getBishopMoves(ChessBoard* board, int square, char color)
{
    std::vector<Move> moves;

    int offsets[] = { -9, -7, 7, 9 };
    int new_square, row, col;

    for (int offset : offsets)
    {
        new_square = square;

        for (int i = 1; i < 8; i++)
        {
            row = getRow(new_square);
            col = getCol(new_square);
            if ((col == 7 and isInArray(offset, { -7, 9 })) || (col == 0 and isInArray(offset, { -9, 7 })) || (row == 7 and isInArray(offset, { 7, 9 })) || (row == 0 and isInArray(offset, { -9, -7 })))
                break;

            new_square += offset;

            if (isValidSquare(new_square))
            {
                if (isPieceThereColor(board, new_square, color))
                    break;
                else if (isPieceThereColor(board, new_square, swapColor(color)))
                {
                    moves.push_back(createMove(square, new_square, pieceToValue(board->pieces[new_square]) - 33, 0, 1));
                    break;
                }
                else
                {
                    moves.push_back(createMove(square, new_square, 0, 0, 1));
                }
            }
            else
                break;
        }
    }

    return moves;
}


std::vector<Move> getRookMoves(ChessBoard* board, int square, char color)
{
    std::vector<Move> moves;

    int offsets[] = { -8, -1, 1, 8 };
    int new_square, row, col;

    for (int offset : offsets)
    {
        new_square = square;

        for (int i = 1; i < 8; i++)
        {
            row = getRow(new_square);
            col = getCol(new_square);
            if ((col == 7 and offset == 1) || (col == 0 and offset == -1) || (row == 7 and offset == 8) || (row == 0 and offset == -8))
                break;

            new_square += offset;

            if (isValidSquare(new_square))
            {
                if (isPieceThereColor(board, new_square, color))
                    break;
                else if (isPieceThereColor(board, new_square, swapColor(color)))
                {
                    moves.push_back(createMove(square, new_square, pieceToValue(board->pieces[new_square]) - 50, 0, 1));
                    break;
                }
                else
                {
                    moves.push_back(createMove(square, new_square, 0, 0, 1));
                }
            }
            else
                break;
        }
    }

    return moves;
}


std::vector<Move> getQueenMoves(ChessBoard* board, int square, char color)
{
    std::vector<Move> moves;

    int offsets[] = { -9, -7, 7, 9, -8, -1, 1, 8 };
    int new_square, row, col;

    for (int offset : offsets)
    {
        new_square = square;

        for (int i = 1; i < 8; i++)
        {
            row = getRow(new_square);
            col = getCol(new_square);
            if ((col == 7 and isInArray(offset, { 1, -7, 9 })) || (col == 0 and isInArray(offset, { -1, -9, 7 })) || (row == 7 and isInArray(offset, { 8, 7, 9 })) || (row == 0 and isInArray(offset, { -8, -9, -7 })))
                break;

            new_square += offset;

            if (isValidSquare(new_square))
            {
                if (isPieceThereColor(board, new_square, color))
                    break;
                else if (isPieceThereColor(board, new_square, swapColor(color)))
                {
                    moves.push_back(createMove(square, new_square, pieceToValue(board->pieces[new_square]) - 90, 0, 1));
                    break;
                }
                else
                {
                    moves.push_back(createMove(square, new_square, 0, 0, 1));
                }
            }
            else
                break;
        }
    }

    return moves;
}


bool isInCheck(ChessBoard* board, char color, int special_square = -1)
{
    int king_square = -1;
    int dir;
    char my_king;

    if (color == 'w')
    {
        my_king = 'K';
        dir = -1;
    }
    else
    {
        my_king = 'k';
        dir = 1;
    }

    if (special_square != -1)
        king_square = special_square;
    else
    {
        for (int i = 0; i < 64; i++)
        {
            if (((color == 'w') && board->pieces[i] == 'K') || ((color == 'b') && board->pieces[i] == 'k'))
            {
                king_square = i;
                break;
            }
        }
    }

    if (king_square == -1)
        return false;



    int pawn_offset1 = 9 * dir;
    int pawn_offset2 = 7 * dir;
    int knight_offsets[] = { -17, -15, -10, -6, 6, 10, 15, 17 };
    int queen_offsets[] = { -9, -7, 7, 9, -8, -1, 1, 8 };
    int new_square, row, col;

    int pawn_place[2];
    pawn_place[0] = king_square + pawn_offset1;
    pawn_place[1] = king_square + pawn_offset1;

    for (int i = 0; i < 2; i++)
    {
        if (isValidSquare(pawn_place[i]) && getRow(pawn_place[i]) == getRow(king_square) + dir)
        {
            if (isPieceThereColor(board, pawn_place[i], swapColor(color)) && toupper(board->pieces[pawn_place[i]]) == 'P')
                return true;
        }
    }

    for (int offset : knight_offsets)
    {
        new_square = king_square + offset;
        if (isValidSquare(new_square) && board->pieces[new_square] != '-' && toupper(board->pieces[new_square]) == 'N')
        {
            if (isupper(board->pieces[new_square]) != (color == 'w'))
            {
                if (abs(getCol(king_square) - getCol(new_square)) > 2)
                {
                    continue;
                }
                return true;
            }
        }
    }

    for (int offset : queen_offsets)
    {
        new_square = king_square;

        for (int i = 1; i < 8; i++)
        {
            row = getRow(new_square);
            col = getCol(new_square);
            //if ((col == 7 and isInArray(offset, { 1, -7, 9 })) || (col == 0 and isInArray(offset, { -1, -9, 7 })) || (row == 7 and isInArray(offset, { 8, 7, 9 })) || (row == 0 and isInArray(offset, { -8, -9, -7 })))
            if ((col == 7 and (offset == 1 || offset == -7 || offset == 9)) || 
                (col == 0 and (offset == -1 || offset == -9 || offset == 7)) || 
                (row == 7 and (offset == 8 || offset == 7 || offset == 9)) || 
                (row == 0 and (offset == -8 || offset == -9 || offset == -7)))
                break;

            new_square += offset;

            if (isValidSquare(new_square))
            {
                if (isPieceThereColor(board, new_square, color) && (board->pieces[new_square] != my_king))
                    break;
                else if (isPieceThereColor(board, new_square, swapColor(color)))
                {
                    if (toupper(board->pieces[new_square]) == 'Q')
                        return true;
                    else if ((offset == -8 || offset == 8 || offset == -1 || offset == 1) && (toupper(board->pieces[new_square]) == 'R'))
                        return true;
                    else if ((offset == -9 || offset == 9 || offset == -7 || offset == 7) && (toupper(board->pieces[new_square]) == 'B'))
                        return true;
                    else if (toupper(board->pieces[new_square]) == 'K' && i == 1)
                        return true;
                    else
                        break;
                }
                else
                    continue;
            }
            else
                break;
        }
    }
    return false;
}


std::vector<Move> getKingMoves(ChessBoard* board, int square, char color)
{
    std::vector<Move> moves;

    int offsets[] = { -9, -7, 7, 9, -8, -1, 1, 8 };
    int new_square, row, col;

    for (int offset : offsets)
    {
        new_square = square;

        row = getRow(new_square);
        col = getCol(new_square);
        if ((col == 7 and isInArray(offset, { 1, -7, 9 })) || (col == 0 and isInArray(offset, { -1, -9, 7 })) || (row == 7 and isInArray(offset, { 8, 7, 9 })) || (row == 0 and isInArray(offset, { -8, -9, -7 })))
            continue;

        new_square += offset;

        if (isValidSquare(new_square))
        {
            if (isPieceThereColor(board, new_square, color))
                continue;
            else
            {
                moves.push_back(createMove(square, new_square, 0, 0, 1));
            }
        }
    }

    if (!isInCheck(board, color))
    {
        bool king_c, queen_c;
        int proper_square, king_cs, queen_cs;
        int empty1, empty2, empty3;
        int empty4, empty5;
        int rook_k, rook_q;
        char my_rook;

        if (color == 'w')
        {
            king_c = board->whiteCanCastleKingSide;
            queen_c = board->whiteCanCastleQueenSide;
            proper_square = 60;
            king_cs = 62;
            queen_cs = 58;

            empty1 = 57;
            empty2 = 58;
            empty3 = 59;

            empty4 = 61;
            empty5 = 62;

            rook_k = 63;
            rook_q = 56;
            my_rook = 'R';
        }
        else
        {
            king_c = board->whiteCanCastleKingSide;
            queen_c = board->whiteCanCastleQueenSide;
            proper_square = 4;
            king_cs = 6;
            queen_cs = 2;

            empty1 = 1;
            empty2 = 2;
            empty3 = 3;

            empty4 = 5;
            empty5 = 6;

            rook_k = 7;
            rook_q = 0;
            my_rook = 'R';
        }

        if (square == proper_square)
        {
            if (king_c && (board->pieces[empty4] == '-') && (board->pieces[empty5] == '-') && (board->pieces[rook_k] == my_rook))
            {
                if (!isInCheck(board, color, empty4) && !isInCheck(board, color, empty5))
                {
                    moves.push_back(createMove(square, king_cs, 0, 0, 1));
                }
            }
            if (queen_c && (board->pieces[empty1] == '-') && (board->pieces[empty2] == '-') && (board->pieces[empty3] == '-') && (board->pieces[rook_q] == my_rook))
            {
                if (!isInCheck(board, color, empty2) && !isInCheck(board, color, empty3))
                {
                    moves.push_back(createMove(square, queen_cs, 0, 0, 1));
                }
            }
        }
    }

    return moves;
}


std::vector<Move> getLegalMovesForAPiece(ChessBoard* board, int square)
{
    std::vector<Move> moves;

    char piece = board->pieces[square];
    char upper_piece = toupper(piece);

    if (piece == '-')
        return moves;

    char color;
    if (isupper(piece) == true)
        color = 'w';
    else
        color = 'b';

    switch (upper_piece)
    {
    case 'P':
        moves = getPawnMoves(board, square, color);
        break;

    case 'N':
        moves = getKnightMoves(board, square, color);
        break;

    case 'B':
        moves = getBishopMoves(board, square, color);
        break;

    case 'R':
        moves = getRookMoves(board, square, color);
        break;

    case 'Q':
        moves = getQueenMoves(board, square, color);
        break;

    case 'K':
        moves = getKingMoves(board, square, color);
        break;
    }

    return moves;
}


ChessBoard movePiece(ChessBoard* board, int square1, int square2)
{
    ChessBoard new_board;

    memcpy(&new_board, board, sizeof(ChessBoard));

    char piece = new_board.pieces[square1];
    char piece_type = toupper(piece);
    char color;
    int dir;

    if (isupper(piece))
    {
        color = 'w';
        dir = -1;
    }
    else
    {
        color = 'b';
        dir = 1;
    }



    new_board.pieces[square2] = new_board.pieces[square1];
    new_board.pieces[square1] = '-';

    new_board.previousCapture = square2;

    new_board.enPassantSquare = -1;
    if (piece_type == 'P')
    {
        if (abs(square2 - square1) == 16)
            new_board.enPassantSquare = (square1 + square2) / 2;

        int row = getRow(square2);

        if (row == 0 && color == 'w')
        {
            new_board.pieces[square2] = 'Q';
        }
        else if (row == 7 && color == 'b')
        {
            new_board.pieces[square2] = 'q';
        }
    }
    else if (piece_type == 'K')
    {
        if (color == 'w')
        {
            new_board.whiteCanCastleKingSide = false;
            new_board.whiteCanCastleQueenSide = false;

            if (square1 == 60 && square2 == 58)
            {
                new_board.pieces[56] = '-';
                new_board.pieces[59] = 'R';
                new_board.whiteCastled = true;
            }
            else if (square1 == 60 && square2 == 62)
            {
                new_board.pieces[63] = '-';
                new_board.pieces[61] = 'R';
                new_board.whiteCastled = true;
            }
        }
        else
        {
            new_board.blackCanCastleKingSide = false;
            new_board.blackCanCastleQueenSide = false;

            if (square1 == 4 && square2 == 2)
            {
                new_board.pieces[0] = '-';
                new_board.pieces[3] = 'R';
                new_board.blackCastled = true;
            }
            else if (square1 == 4 && square2 == 6)
            {
                new_board.pieces[7] = '-';
                new_board.pieces[5] = 'R';
                new_board.whiteCastled = true;
            }
        }
    }
    else if (piece_type == 'R')
    {
        if (color == 'w')
        {
            if (square1 == 56)
                new_board.whiteCanCastleQueenSide = false;
            if (square1 == 63)
                new_board.whiteCanCastleKingSide = false;
        }
        else
        {
            if (square1 == 0)
                new_board.blackCanCastleQueenSide = false;
            if (square1 == 7)
                new_board.blackCanCastleKingSide = false;
        }
    }

    new_board.turn = swapColor(new_board.turn);


    return new_board;
}


std::vector<Move> getLegalMovesFor(ChessBoard* board, int full_depth, Move tt_move)
{
    std::vector<Move> moves;
    std::vector<Move> legal_moves;

    for (int i = 0; i < 64; i++)
    {
        if (board->pieces[i] != '-' && isPieceOfColor(board->pieces[i], board->turn))
        {
            std::vector<Move> pieceMoves = getLegalMovesForAPiece(board, i);
            moves.insert(moves.end(), pieceMoves.begin(), pieceMoves.end());
        }
    }

    for (int i = 0; i < moves.size(); i += 1)
    {
        ChessBoard new_board = movePiece(board, moves[i].sq1, moves[i].sq2);
        if (!isInCheck(&new_board, board->turn))
        {
            if (board->previousCapture == moves[i].sq2)
                moves[i].contest = 1;
            moves[i].score = history_table[std::to_string(board->turn) + moveToString(moves[i])];
            legal_moves.push_back(moves[i]);
        }
    }

    std::sort(legal_moves.begin(), legal_moves.end(), [full_depth, tt_move](const Move& a, const Move& b) {

        if (tt_move.sq1 != -1 && tt_move.sq2 != -1)
        {
            if (tt_move.sq1 == a.sq1 && tt_move.sq2 == a.sq2)
                return true;
            if (tt_move.sq1 == b.sq1 && tt_move.sq2 == b.sq2)
                return false;
        }
        else if (a.contest != b.contest)
            return a.contest > b.contest;
        else if (a.capture != b.capture)
            return a.capture > b.capture;
        else {
            for (int i = 0; i < 2; ++i) {
                if (killer_moves[full_depth][i].sq1 == a.sq1 && killer_moves[full_depth][i].sq2 == a.sq2)
                    return true;
                if (killer_moves[full_depth][i].sq1 == b.sq1 && killer_moves[full_depth][i].sq2 == b.sq2)
                    return false;
            }
        }

        if (a.piece_class != b.piece_class)
            return a.piece_class > b.piece_class;
        else
            return a.score > b.score;
    });

    return legal_moves;
}


int evalBoard(ChessBoard* board)
{
    int white_score = 0;
    int black_score = 0;

    int white_pieces = 0;
    int black_pieces = 0;
    int white_pawns = 0;
    int black_pawns = 0;

    int king_safety = 100;

    int white_king_i = -1;
    int black_king_i = -1;

    bool black_queen = false;
    bool white_queen = false;


    if (board->blackCanCastleKingSide)
        black_score += 30;
    if (board->blackCanCastleQueenSide)
        black_score += 30;
    if (board->blackCastled)
        black_score += 90;

    if (board->whiteCanCastleKingSide)
        white_score += 30;
    if (board->whiteCanCastleQueenSide)
        white_score += 30;
    if (board->whiteCastled)
        white_score += 90;



    for (int i = 0; i < 64; i++)
    {
        switch (board->pieces[i])
        {
        case 'P':
            white_score += 100 + pawn_list[i];
            white_pawns += 1;
            break;
        case 'p':
            black_score += 100 + pawn_list[63-i];
            black_pawns += 1;
            break;
        case 'N':
            white_score += 320 + knight_list[i];
            white_pieces += 1;
            break;
        case 'n':
            black_score += 320 + knight_list[63-i];
            black_pieces += 1;
            break;
        case 'B':
            white_score += 340 + bishop_list[i];
            white_pieces += 1;
            break;
        case 'b':
            black_score += 340 + bishop_list[63-i];
            black_pieces += 1;
            break;
        case 'R':
            white_score += 500 + rook_list[i];
            white_pieces += 1;
            break;
        case 'r':
            black_score += 500 + rook_list[63-i];
            black_pieces += 1;
            break;
        case 'Q':
            white_score += 900 + queen_list[i];
            white_queen = true;
            break;
        case 'q':
            black_score += 900 + queen_list[63-i];
            black_queen = true;
            break;
        case 'K':
            white_score += king_safety;
            white_pieces += 1;
            break;
        case 'k':
            black_score += king_safety;
            black_pieces += 1;
            break;
        }
    }

    if (black_king_i != -1 && white_king_i != -1)
    {
        if ((!white_queen && !black_queen) || (white_pieces <= 3 && black_pieces <= 3)) {
            black_score += king_end_list[63 - black_king_i];
            white_score += king_end_list[black_king_i];
        }
        else {
            black_score += king_middle_list[63 - black_king_i];
            white_score += king_middle_list[black_king_i];
        }
    }
    

    int diff = white_score - black_score;

    int result = white_score - black_score;

    if (abs(diff) > 130)
    {
        diff *= 0.2f;
        result += diff;
    }

    return result;
}


ChessBoard makeNullMove(ChessBoard* board)
{
    ChessBoard new_board;
    for (int i = 0; i < 64; i++)
    {
        new_board.pieces[i] = board->pieces[i];
    }
    new_board.turn = swapColor(board->turn);
    return new_board;
}


MoveEval negamax(ChessBoard* board, int alpha, int beta, int depth, int big_depth, int reduced_by = 1)
{
    int dir = (board->turn == 'w') ? 1 : -1;
    //std::string hashKey = generateHashKey(board);
    std::string hashKey = generateHashKey(board);
    std::string simple_hash = generateHashKeySimple(board);

    if (opening_table.find(simple_hash) != opening_table.end()) {
        return createMoveEval(getRandomValue(opening_table[simple_hash]), 0);
    }

    Move tt_move = createMove(-1, -1);

    if (transpositionTable.find(hashKey) != transpositionTable.end()) {

        if (transpositionTable[hashKey].depth >= depth) {
            // Entry found in the transposition table
            TranspositionEntry entry = transpositionTable[hashKey];
            if (entry.depth >= depth) {
                // Use the stored value if it's at least as deep as the current depth
                if (entry.depth == depth)
                {
                    return createMoveEval(entry.move, entry.score);
                }
                if (entry.depth < depth)
                {
                    return createMoveEval(entry.move, entry.score * dir);
                }
            }
        }
        else {
            tt_move = transpositionTable[hashKey].move;
        }
        
    }

    std::vector<Move> moves = getLegalMovesFor(board, big_depth, tt_move);

    trees_searched += 1;

    bool increased = false;
    char other_color = swapColor(board->turn);
    bool me_in_check = isInCheck(board, board->turn);
    //bool other_in_check;
    Move best_move = createMove(-1, -1);
    Move blank_move = createMove(-1, -1);

    if (me_in_check)
    {
        increased = true;
        depth += reduced_by;
    }

    if (moves.size() == 0)
    {
        if (me_in_check)
            return createMoveEval(blank_move, -(MATE_VALUE + big_depth));
        else
            return createMoveEval(blank_move, 0);
    }
    //other_in_check = isInCheck(board, other_color);

    if (depth == 0 || big_depth == 0)
    {
        int val = evalBoard(board);
        return createMoveEval(blank_move, val * dir);
    }

    int best_score = -MAX_VALUE;
    int move_count = moves.size();
    int move_iterator = 0;


    //
    if (depth > 1 && !isInCheck(board, swapColor(board->turn)) && depth > 2) {
        ChessBoard new_board_n = makeNullMove(board);
        MoveEval null_eval = negamax(&new_board_n, -beta, -beta + 1, depth - 3, big_depth - 3);
        null_eval.eval = -null_eval.eval; // negate the score
        if (null_eval.eval >= beta) {
            return createMoveEval(createMove(-1, -1), null_eval.eval);
        }
    }


    for (const Move& move : moves)
    {
        move_iterator += 1;
        ChessBoard new_board = movePiece(board, move.sq1, move.sq2);
        MoveEval eval;
        if (!increased && moves.size() < 5)
            eval = negamax(&new_board, -beta, -alpha, depth, big_depth - 1);
        else {
            if (move_iterator > 2 && move.capture == 0 && depth > 2 && !me_in_check)
            {
                eval = negamax(&new_board, -beta, -alpha, depth - 2, big_depth - 1, 2);
                if (alpha < -eval.eval)
                    eval = negamax(&new_board, -beta, -alpha, depth - 1, big_depth - 1);
            }
            else
                eval = negamax(&new_board, -beta, -alpha, depth - 1, big_depth - 1);
        }
            
        
        int score = -eval.eval;
        if (score > best_score)
        {
            best_score = score;
            best_move = move;
        }
        alpha = std::max(alpha, best_score);
        if (beta <= alpha)
        {
            if (move.capture == 0)
            {
                updateKillerMove(depth, move);
                insertToHistory(move, board->turn, big_depth);
            }
            break; // Beta cutoff
        }
    }

    // Store the current position in the transposition table
    transpositionTable[hashKey] = { best_score, depth, best_move };

    return createMoveEval(best_move, best_score);
}



void parseOpenings()
{
    std::ifstream file("fen_results.txt");
    if (!file.is_open()) {
        std::cerr << "Error opening file!" << std::endl;
        return;
    }

    std::string line;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string firstPart, secondPart, secondPartFirstHalf, secondPartSecondHalf;

        // Read everything until the last space
        std::size_t lastSpacePos = line.find_last_of(' ');
        if (lastSpacePos != std::string::npos) {
            firstPart = line.substr(0, lastSpacePos);
        }

        // Last 4 characters
        secondPart = line.substr(lastSpacePos + 1);

        if (secondPart.length() >= 2) {
            secondPartFirstHalf = secondPart.substr(0, secondPart.length() / 2);
            secondPartSecondHalf = secondPart.substr(secondPart.length() / 2);
        }

        ChessBoard board = fenToChessBoard(firstPart);
        std::string simple_hash = generateHashKeySimple(&board);

        int first, second;
        first = getSquareIndex(secondPartFirstHalf);
        second = getSquareIndex(secondPartSecondHalf);

        Move created = createMove(first, second);
        opening_table[simple_hash].push_back(created);
    }

    file.close();
}



int main(int argc, char* argv[])
{
    std::string input_string;

    ChessBoard board;
    MoveEval mEval;

    REGULAR_DEPTH = 8;
    MAX_DEPTH = 12;
    
    parseOpenings();


    //input_string = "rnbqkb1r/pp2pppp/2p5/6B1/2pPN3/8/PP3PPP/R2QKBNR b KQkq - 0 6";
    //board = fenToChessBoard(input_string);

    //int score = evalBoard(&board);

    //std::cout << score << std::endl;

    //std::vector<Move> moves = getLegalMovesFor(&board, MAX_DEPTH, createMove(-1, -1));

    //for (auto move : moves) {
        //std::cout << move.sq1 << " " << move.sq2 << std::endl;
    //}


    //Input depth on runtime
    //std::cout << "Regular depth: ";
    //std::getline(std::cin, temp);
    //REGULAR_DEPTH = stoi(temp);

    //std::cout << "Max depth: ";
    //std::getline(std::cin, temp);
    //MAX_DEPTH = stoi(temp);

    while (true) {
        transpositionTable.clear();
        history_table.clear();
        for (int i = 0; i < 20; ++i) {
            for (int j = 0; j < 2; ++j) {
                killer_moves[i][j] = createMove(-1, -1);
            }
        }
        trees_searched = 0;


        std::cout << "Enter FEN: ";
        std::getline(std::cin, input_string);
        
        board = fenToChessBoard(input_string);

        // Iterative deepening
        int starting_reduction = REGULAR_DEPTH - 2;
        for (int i = starting_reduction; i > 0; i--)
            negamax(&board, -MAX_VALUE, MAX_VALUE, REGULAR_DEPTH - i, MAX_DEPTH - i);

        mEval = negamax(&board, -MAX_VALUE, MAX_VALUE, REGULAR_DEPTH, MAX_DEPTH);

        std::cout << "move: " << getSquareNotation(mEval.move.sq1) << "->" << getSquareNotation(mEval.move.sq2) << " eval: " << mEval.eval << std::endl;
        std::cout << "nodes searched: " << trees_searched << std::endl;
    }

    return 0;
}