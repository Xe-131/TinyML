import re

# 文件路径
file1_path = r'./MODEL_QUANT_TFLITE_8000.cc'
file2_path = r'../ESP_Arduino_Code/MODEL_QUANT_TFLITE_8000.h'

# 正则表达式，用于匹配（带 const 的）unsigned char 和 unsigned int 变量定义，支持多行
variable_pattern1 = re.compile(r'(unsigned\s+(char|int)\s+\w+\s*\[\]\s*=\s*\{.*?\};|unsigned\s+(char|int)\s+\w+\s*=\s*[^;]+;)', re.DOTALL)

def extract_variables(file_path):
    """
    从文件中提取前两个变量定义
    """
    variables = []
    with open(file_path, 'r', encoding='utf-8') as file:
        content = file.read()  # 读取整个文件的内容
        matches = variable_pattern1.findall(content)  # 查找所有匹配的变量定义
        if matches:
            for match in matches:
                variable = match[0].strip()
                variables.append(variable)
                # print()
                # print(f"Found variable: {variable}")  # 调试输出找到的变量
                if len(variables) == 2:  # 只需要两个变量
                    break
    return variables

def replace_variables_in_header(file_path, variables):
    with open(file_path, 'r', encoding='utf-8') as file:
        content = file.read()

    # 找到第一个和第二个变量的结束位置
    first_end_index = content.find('};', content.find('const ')) + 2
    second_start_index = content.find('const ', first_end_index)
    second_end_index = content.find(';', second_start_index) + 2

    # 用新变量替换
    content = (variables[0] + '\n' + content[first_end_index:second_start_index] +
               variables[1] + '\n' + content[second_end_index:])

    with open(file_path, 'w', encoding='utf-8') as file:
        file.write(content)

def main():
    # 从 A.cpp 中提取变量
    variables_from_file1 = extract_variables(file1_path)
    # 替换
    variables_from_file1[0] = variables_from_file1[0].replace('__MODELS_MODEL_QUANT_TFLITE_8000', 'MODEL_QUANT_TFLITE_8000_FLASH_1').replace('unsigned char', 'const unsigned char')
    variables_from_file1[1] = variables_from_file1[1].replace('__MODELS_MODEL_QUANT_TFLITE_8000_len', 'MODEL_QUANT_TFLITE_8000_len_1').replace('unsigned int', 'const unsigned int')

    # print("提取到的变量：")
    # print(variables_from_file1)


    # 用 A.cpp 的变量替换 B.cpp 前两个变量
    replace_variables_in_header(file2_path, variables_from_file1)
    
    print()
    print(f"Successfully copied variables from {file1_path} to {file2_path}.")
    print()

if __name__ == "__main__":
    main()
