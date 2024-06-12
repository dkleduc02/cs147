import pandas as pd
import matplotlib.pyplot as plt

def visualize(df: pd.DataFrame):

    # change data to datatime
    df.iloc[:, 0] = pd.to_datetime(df.iloc[:, 0])

    # create figure
    plt.figure(figsize=(12, 6))

    # plot timestamp by temperature
    plt.plot(df.iloc[:, 0], df.iloc[:, 1], marker='o', linestyle='-', color='b')
    
    # plot settings
    plt.title('Temperature Over Time')
    plt.xlabel('Timestamp')
    plt.ylabel('Temperature (in Celsius)')
    plt.grid(True)
    plt.xticks(rotation=45)
    plt.tight_layout()
    plt.show()


    df.iloc[:, 0] = pd.to_datetime(df.iloc[:, 0])
    # map Yes to 1 and No to 0
    df.iloc[:, 2] = df.iloc[:, 2].str.lower().map({'on': 1, 'off': 0})

    # create figure
    plt.figure(figsize=(12, 6))

    # plot timestamp by motion detected boolean
    plt.plot(df.iloc[:, 0], df.iloc[:, 2], marker='o', linestyle='-', color='b')
    
    # plot settings
    plt.title('Motion Detection Over Time')
    plt.xlabel('Timestamp')
    plt.ylabel('Motion Detected (1 = ON, 0 = OFF)')
    plt.grid(True)
    plt.xticks(rotation=45)
    plt.tight_layout()
    plt.show()

if __name__ == '__main__':
    df = pd.read_csv('data.csv')
    visualize(df)


    

