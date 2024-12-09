import matplotlib.pyplot as plt

def read_data(filename):
    x_values = []
    y_values = []
    
    try:
        with open(filename, 'r') as file:
            for line in file:
                x, y = line.strip().split()
                try:
                    x_value = float(x)
                    y_value = float(y)
                    x_values.append(x_value)
                    y_values.append(y_value)
                except ValueError:
                    print(f"Skipping invalid data: {line.strip()}")
    
    except FileNotFoundError:
        print(f"File '{filename}' not found.")
    
    return x_values, y_values

def plot_data(x_values, y_values):
    plt.figure(figsize=(10, 6))
    plt.plot(x_values, y_values, marker='o')
    plt.xlabel('x')
    plt.ylabel('y')
    plt.title('y(x)')
    plt.grid(True)
    plt.show()


input_file = 'Calculations.txt'


x, y = read_data(input_file)

if x and y:
    plot_data(x, y)
else:
    print("No valid data to plot.")
    
