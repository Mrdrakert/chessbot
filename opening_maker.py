import chess

# Function to process moves and generate FEN positions
def generate_fen_positions(moves_list):
    fen_positions = []
    for moves in moves_list:
        board = chess.Board()  # Initialize the chess board
          # Save the initial FEN position
        fens = []
        for move in moves:
            fens.append((board.fen(), move))  # Save FEN position and move
            board.push_san(move)  # Perform each move
        fen_positions.append(fens)
    return fen_positions

# Read moves from the file
with open('openings.txt', 'r') as file:
    moves_list = [line.strip().split() for line in file]

# Generate FEN positions
fen_positions = generate_fen_positions(moves_list)

with open('fen_results.txt', 'w') as output_file:
    for fens in fen_positions:
        for fen, move in fens:
            output_file.write(f"{fen} - {move}\n")
