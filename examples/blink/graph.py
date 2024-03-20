import matplotlib.pyplot as plt
import pandas as pd

fig, axes = plt.subplots(1, 2, figsize=(12, 6))

for k, n in enumerate([10, 50]):
    for mode in ["normal", "syscall"]:
        df = pd.read_csv('data/{}_{}.csv'.format(mode, n))
        df = df.sort_values(by='time')
        df = df.reset_index(drop=True)
        axes[k].plot(df['time']*1000, label=mode, color='C0' if mode == 'normal' else 'C1')
        mean_time = df['time'].mean()*1000
        axes[k].axhline(y=mean_time, linestyle='--', color='C0' if mode == 'normal' else 'C1', label='{} mean'.format(mode))
        axes[k].set_title('Matrix size = {}x{}'.format(n, n))
        axes[k].set_xlabel('Iteration')
        axes[k].set_ylabel('Time (ms)')
        axes[k].legend()
        axes[k].grid(True)

plt.tight_layout()
plt.show()
