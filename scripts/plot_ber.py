import pandas as pd
import matplotlib.pyplot as plt
import os
from cycler import cycler


csv_filename = "../build/simulation_results_snr.csv"
output_plot_filename_optional = "../ber_vs_snr_plot.png"

plot_title = "BER vs. SNR (Eb/N0)"
x_label = "SNR (Eb/N0) [dB]"
y_label = "Bit Error Rate (BER)"
plot_style = 'dark_background'

bright_colors = [
    '#00FFFF',
    '#FFFF00',
    '#FF00FF',
    '#00FF00',
    '#FFA500',
    '#FF69B4',
]
# ---------------------------------------------------

def plot_ber_curves(data_file, save_to_file=False, output_filename="ber_plot.png"):

    if not os.path.exists(data_file):
        alt_data_file = "../simulation_results_snr.csv"
        if os.path.exists(alt_data_file):
            print(f"Info: Data file found at {alt_data_file} instead of {data_file}.")
            data_file = alt_data_file
        else:
            print(f"Error: Data file not found at {data_file} or {alt_data_file}")
            return
    try:
        df = pd.read_csv(data_file)
        print("Data loaded successfully:")
        print(df.head())
    except Exception as e:
        print(f"Error reading or processing CSV file: {e}")
        return

    required_columns = ['Modulation', 'SNR_dB', 'BER']
    if not all(col in df.columns for col in required_columns):
        print(f"Error: CSV file must contain columns: {required_columns}")
        print(f"Found columns: {df.columns.tolist()}")
        return

    min_ber_display = 1e-9
    df['BER'] = df['BER'].replace(0.0, min_ber_display)

    try:
        plt.style.use(plot_style)
        print(f"Using plot style: {plot_style}")
    except OSError:
        print(f"Warning: Style '{plot_style}' not found. Using default style.")

    fig, ax = plt.subplots(figsize=(10, 7))

    modulations = df['Modulation'].unique()
    num_modulations = len(modulations)
    ax.set_prop_cycle(cycler(color=bright_colors[:num_modulations]))

    mod_order = {'QPSK': 0, 'QAM16': 1, 'QAM64': 2}
    modulations = sorted(modulations, key=lambda x: mod_order.get(x, 99))

    for mod_type in modulations:
        mod_data = df[df['Modulation'] == mod_type].copy()
        mod_data = mod_data.sort_values(by='SNR_dB')

        x_values = mod_data['SNR_dB'].values
        y_values = mod_data['BER'].values

        ax.plot(x_values, y_values,
                 marker='o',
                 markersize=5,
                 linestyle='-',
                 linewidth=1.5,
                 label=mod_type)

    ax.set_xlabel(x_label)
    ax.set_ylabel(y_label)
    ax.set_title(plot_title)
    ax.set_yscale('log')

    # ax.set_xscale('log')

    min_ber_in_data = df[df['BER'] > 0]['BER'].min()
    if pd.notna(min_ber_in_data) and min_ber_in_data > 0:
        ax.set_ylim(bottom=max(min_ber_in_data * 0.1, 1e-7), top=1.1)
    else:
        ax.set_ylim(bottom=min_ber_display * 0.1, top=1.1)


    ax.grid(True, which='both', linestyle=':', linewidth=0.5, alpha=0.6)
    ax.legend(loc='lower left')
    fig.tight_layout()

    if save_to_file:
        try:
            plt.savefig(output_filename, dpi=300)
            print(f"Plot saved successfully to {output_filename}")
        except Exception as e:
            print(f"Error saving plot: {e}")
    else:
        print("Displaying plot...")
        plt.show()

if __name__ == "__main__":
    script_dir = os.path.dirname(os.path.abspath(__file__))
    os.chdir(script_dir)
    plot_ber_curves(csv_filename, save_to_file=False, output_filename=output_plot_filename_optional)