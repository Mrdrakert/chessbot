import csv
import chess



def parse_moves(move_string):
    return move_string.split()

def parse_moves(move_string):
    return move_string.split()

def read_csv(filename):
    move_sequences = []
    with open(filename, newline='') as csvfile:
        reader = csv.DictReader(csvfile)
        for row in reader:
            moves = parse_moves(row['moves'])
            move_sequences.append(moves)
    return move_sequences

def filter_move_sequences(move_sequences):
    filtered_sequences = []
    seen = set()  # Keep track of seen move sequences
    # Sort the move sequences so that the shorter ones are at the end
    sorted_sequences = sorted(move_sequences, key=len, reverse=True)
    for moves in sorted_sequences:
        # Check if a longer counterpart of the move sequence has been seen before
        if not any(tuple(ms) in seen for ms in [moves[i:] for i in range(len(moves))]):
            filtered_sequences.append(moves)
            seen.update(tuple(moves) for moves in [moves[i:] for i in range(len(moves))])
    return filtered_sequences

def main():
    filename = 'openings_sheet.csv'  # Replace 'your_csv_file.csv' with the path to your CSV file
    move_sequences = read_csv(filename)
    filtered_sequences = filter_move_sequences(move_sequences)

    finished_lines = []

    for moves in filtered_sequences:
        try:
            move = []
            board = chess.Board()
            for m in moves:
                uci_text = board.push_san(m).uci()
                move.append(uci_text)
            finished_lines.append(move)
        except:
            print("Error in line: ", moves)

    #save to file each line in a new line separated by spaces
    with open('openings.txt', 'w') as f:
        for item in finished_lines:
            f.write("%s\n" % ' '.join(item))


if __name__ == "__main__":
    main()
