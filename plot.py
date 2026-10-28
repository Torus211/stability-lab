import csv
from pathlib import Path

import matplotlib.pyplot as plt

folder = Path(__file__).resolve().parent


def read_csv(name):
    with (folder / name).open(newline="") as file:
        rows = [{key: float(value) for key, value in row.items()}
                for row in csv.DictReader(file)]
    if not rows:
        raise ValueError(f"Файл {name} пуст")
    return rows


try:
    trajectory = read_csv("trajectory.csv")
    critical = read_csv("critical.csv")
    times = [row["t"] for row in trajectory]
    output = [row["x"] for row in trajectory]
    damping = [row["a1"] for row in critical]
except (OSError, ValueError, TypeError, KeyError) as error:
    raise SystemExit(f"Не удалось прочитать данные: {error}. Сначала запустите C++ в папке проекта.")

fig, axes = plt.subplots(1, 2, figsize=(12, 5))
axes[0].plot(times, output)
axes[0].set(title="Переходный процесс", xlabel="Время t", ylabel="Выход x(t)")

for column in critical[0]:
    if column == "a1":
        continue
    values = [row[column] if row[column] >= 0 else float("nan") for row in critical]
    axes[1].plot(damping, values, label="a2 = " + column.removeprefix("tau_a2_"))
axes[1].set(title="Критическое запаздывание", xlabel="Коэффициент a1",
            ylabel="Критическая задержка τ")
axes[1].legend()

for ax in axes:
    ax.grid(True, alpha=0.3)

fig.tight_layout()
fig.savefig(folder / "graphs.png", dpi=150)
print("Графики сохранены в graphs.png")
