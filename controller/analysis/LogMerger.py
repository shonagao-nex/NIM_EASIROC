import pandas as pd
import matplotlib.pyplot as plt
from matplotlib.backends.backend_pdf import PdfPages
from datetime import datetime, timedelta
import glob
import time
import os
import signal
from datetime import datetime
from io import StringIO

summary_file = "data/summary.log"
running = True
file_state = {}

def load_and_append_logs(first_run=False):
    global file_state

    log_files = sorted(glob.glob("data/run_*.log"))
    new_rows = []

    for file in log_files:
        try:
            mtime = os.path.getmtime(file)
            prev_mtime, prev_lines = file_state.get(file, (0, 0))

            # Check if the file is new or has been modified
            if first_run or mtime > prev_mtime:
                with open(file, "r") as f:
                    lines = f.readlines()

                header = lines[0].strip()
                data_lines = lines[1:]

                # Only process new lines
                new_data = data_lines[prev_lines:]
                file_state[file] = (mtime, len(data_lines))

                for line in new_data:
                    new_rows.append(line.strip())

                print(f"Complete: {file}")

        except Exception as e:
            print(f"Failed to read {file}: {e}")

    # If new data exists, handle output
    if new_rows:
        if first_run or not os.path.exists(summary_file):
            # Write header + all new data
            with open(summary_file, "w") as f:
                if log_files:
                    with open(log_files[0], "r") as ref:
                        f.write(ref.readline())  # Write header
                for line in new_rows:
                    f.write(line + "\n")
            print(f"Initial run: summary.log created with {len(new_rows)} row(s).")
        else:
            # Append only new data
            with open(summary_file, "a") as f:
                for line in new_rows:
                    f.write(line + "\n")
            print(f"Appended {len(new_rows)} new row(s) to summary.log.")

    return bool(new_rows)


SUMMARY_FILE = "data/summary.log"
OUTPUT_PDF = "pdfs/summary_plots.pdf"
LAST_N_LINES = 2000  # 読み込む最大行数
TIME_WINDOW_HOURS = 12
MAX_POINTS = 1000  # 描画点数の上限

# Y軸範囲
y_ranges = {
    "HV(V)": (0, 62),
    "current(uA)": (0, 20),
    "T1(degC)": (10, 40),
}

# 表示するグラフ項目
columns_list = [
    ("HV(V)",),
    ("current(uA)",),
    ("T1(degC)",),
]

# 高速読み込み：末尾から最大N行だけ読む
def load_recent_lines(filepath, last_n=5000):
    with open(filepath, "r") as f:
        lines = f.readlines()

    if len(lines) <= 1:
        raise ValueError("summary.log does not contain enough data.")

    header = lines[0]
    data_lines = lines[1:]
    lines_to_read = data_lines[-last_n:]
    return pd.read_csv(StringIO("".join([header] + lines_to_read)))

# ダウンサンプリング
def downsample(df, max_points=MAX_POINTS):
    if len(df) > max_points:
        return df.iloc[::len(df)//max_points]
    return df

# メイン処理
def generate_pdf():
    print("Loading recent summary.log data...")
    df = load_recent_lines(SUMMARY_FILE)
    df['timestamp'] = pd.to_datetime(df['timestamp']).dt.tz_localize(None)

    start_time = datetime.now() - timedelta(hours=TIME_WINDOW_HOURS)
    df = df[df['timestamp'] >= start_time]
#    df = downsample(df)

    print(f"Plotting {len(df)} points over the last {TIME_WINDOW_HOURS} hours...")

    with PdfPages(OUTPUT_PDF) as pdf:
        fig, axs = plt.subplots(3, 1, figsize=(8.27, 11.69))  # A4
        fig.subplots_adjust(hspace=0.4)
        fig.suptitle(f"HV / Current / T1 - Last {TIME_WINDOW_HOURS} Hours", fontsize=14)

        for ax, cols in zip(axs, columns_list):
            for col in cols:
                ax.plot(df["timestamp"], df[col], label=col)
                ax.set_ylabel(col)
                if col in y_ranges:
                    ax.set_ylim(y_ranges[col])
            ax.set_xlabel("Timestamp")
            ax.legend()
            ax.grid(True)
            ax.tick_params(axis='x', labelrotation=45)

        pdf.savefig(fig)
        plt.close(fig)

    print(f"PDF saved to: {OUTPUT_PDF}")


if __name__ == "__main__":

    updated = load_and_append_logs(first_run=True)
#    if updated:
#        generate_pdf()

    try:
#        generate_pdf()
        while running:
            updated = load_and_append_logs()
            print(datetime.now().strftime("%Y-%d-%m %H:%M:%S") + " - Watching for new or updated log files... Press Ctrl+C or send SIGTERM to stop.")
            time.sleep(1)
#            if updated:
#                generate_pdf()

            time.sleep(120)
    except KeyboardInterrupt:
        print("Stopped watching for log files.")
    finally:
        print("Watcher stopped.")

