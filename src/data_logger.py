import json
import os
import sys
import urllib.request
from datetime import datetime
from pathlib import Path
from typing import *


def gather_data(sensor: List) -> List[Dict]:
    current_datetime = datetime.now().strftime('%Y-%m-%d %H:%M:%S')
    output_data = []

    for se in sensor:
        current_dict = {'datetime': current_datetime, 'room': se['name']}
        for si in se['signals']:
            url = f"http://{se['address']}/{si}"
            try:
                response = urllib.request.urlopen(url)
                value = response.read().decode("utf-8")
            except urllib.error.URLError:
                value = '###'
            current_dict[si] = value
        output_data.append(current_dict)

    return output_data


def log_data(data: List[Dict], db_path: str) -> NoReturn:
    with open(db_path, 'a+') as database:
        for room in data:
            database.write(','.join(room.values()) + '\n')

def copy_last_n_rows(source_file: str, dest_file: str, num_rows: int) -> NoReturn:
    """
    Efficiently copies the last `num_rows` lines from a CSV file.

    :param source_file: Path to the source CSV file.
    :param dest_file: Path to the destination CSV file.
    :param num_rows: Number of rows to copy from the end of the source file.
    """
    with open(source_file, 'rb') as src:
        # Seek to the end of the file
        src.seek(0, os.SEEK_END)
        end_pos = src.tell()
        buffer_size = 8192  # Size of the buffer to read

        # Initialize a list to store lines
        lines = []

        # Read from the end of the file in chunks
        while end_pos > 0 and len(lines) < num_rows:
            start_pos = max(end_pos - buffer_size, 0)
            src.seek(start_pos)
            buffer = src.read(end_pos - start_pos)
            end_pos = start_pos

            # Decode buffer and split into lines
            buffer = buffer.decode('utf-8', errors='ignore')  # Decode buffer to string
            chunk_lines = buffer.splitlines()

            # Prepend the lines from this chunk to the list
            lines = chunk_lines + lines

        # Only keep the last `num_rows` rows
        lines = lines[-num_rows:]

    # Write the collected lines to the destination file
    with open(dest_file, 'w') as dst:
        dst.write('\n'.join(lines) + '\n')


if __name__ == '__main__':

    # Check if a command line argument is provided
    if len(sys.argv) != 2:
        print(f"Usage: python3 src/{Path(__file__).name} <json_file_path>")
        sys.exit(1)

    json_file_path = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', sys.argv[1]))

    with open(json_file_path) as json_file:
        json_data = json.load(json_file)
        full_db_path = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', json_data['metadata']['full_database_path']))
        full_db_file = os.path.abspath(os.path.join(full_db_path, json_data['metadata']['full_database_name']))
        small_db_path = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', json_data['metadata']['small_database_path']))
        small_db_file = os.path.abspath(os.path.join(small_db_path, json_data['metadata']['small_database_name']))
        num_rows_to_copy = json_data['metadata']['num_rows_to_copy']
        sensors = json_data['servers']

    data = gather_data(sensors)
    log_data(data=data, db_path=full_db_file)
    copy_last_n_rows(source_file=full_db_file, dest_file=small_db_file, num_rows=num_rows_to_copy)
