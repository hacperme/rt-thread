def get_api_hash(src):
    h = 0
    for char in src:
        # Perform operations within 32-bit range
        h = (5527 * h + 7 * ord(char)) & 0xFFFFFFFF
        v = h & 0x0000FFFF
        h = (h ^ (v * v)) & 0xFFFFFFFF  # Ensure h remains within 32 bits
    return h

def main():
    api_list = []
    with open('api_list.txt', 'r') as file:
        for line in file:
            api_name = line.strip()  # Remove newline character
            hash_value = get_api_hash(api_name)
            formatted_hash = "{:08x}".format(hash_value)
            api_list.append(f"{{0x{formatted_hash}, {api_name}}}")

    print("static rt_api_list_t rt_api_list[] = {")
    for item in api_list:
        print(f"    {item},")
    print("};")

if __name__ == "__main__":
    main()