import json
import os
import sys

def read_json_file(filepath):
    with open(filepath, 'r', encoding='utf-8') as file:
        return [json.loads(line) for line in file]

def write_json_file(filepath, data):
    with open(filepath, 'w', encoding='utf-8') as file:
        for item in data:
            file.write(json.dumps(item, ensure_ascii=False) + '\n')

def merge_and_deduplicate(json_files):
    combined_data = []
    seen = set()
    
    for file in json_files:
        data = read_json_file(file)
        for item in data:
            key = (item['saddr'], item['sport'])
            if key not in seen:
                seen.add(key)
                combined_data.append({
                    "saddr": item['saddr'],
                    "sport": item['sport'],
                    "fingerprint": item['fingerprint'],
                    "window": item['window'],
                    "data": item['data'],
                })
    
    return combined_data

if __name__ == "__main__":
    if len(sys.argv) < 3:
        print("Usage: python combine_results.py <output_file> <json_file1> <json_file2> ... <json_fileN>")
        sys.exit(1)
    
    output_file = sys.argv[1]
    json_files = sys.argv[2:]
    
    combined_data = merge_and_deduplicate(json_files)
    write_json_file(output_file, combined_data)
