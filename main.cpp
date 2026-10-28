#include <cmath>
#include <fstream>
#include <iostream>
#include <vector>
using namespace std;

// -1 означает отсутствие конечной границы задержки.
double criticalDelay(double a2, double a1, double a0, double b0, double k) {
    double gain = k * b0;
    double linear = a1 * a1 - 2 * a0 * a2;
    double discriminant = linear * linear - 4 * a2 * a2 * (a0 * a0 - gain * gain);
    if (discriminant < 0) return -1;
    double frequencySquared = (-linear + sqrt(discriminant)) / (2 * a2 * a2);
    if (frequencySquared <= 0) return -1;
    double frequency = sqrt(frequencySquared);
    return atan2(a1 * frequency, a2 * frequencySquared - a0) / frequency;
}

int main() {
    // a2*x'' + a1*x' + a0*x = b0*k*(1 - x(t - tau)).
    const double a2 = 1, a1 = 1, a0 = 1, b0 = 1, k = 2;
    const double dt = 0.005, stop = 100;
    double tau;
    cout << "Критическая задержка: " << criticalDelay(a2, a1, a0, b0, k) << '\n';
    cout << "Введите задержку tau: ";
    if (!(cin >> tau) || !isfinite(tau) || tau < 0 || tau > stop) {
        cerr << "Нужна задержка от 0 до " << stop << '\n';
        return 1;
    }

    int steps = lround(stop / dt);
    int delay = lround(tau / dt);
    cout << "Задержка на сетке: " << delay * dt << '\n';
    vector<double> x(steps + 1, 0);
    ofstream trajectory("trajectory.csv"), critical("critical.csv");
    if (!trajectory || !critical) {
        cerr << "Не удалось создать CSV-файлы\n";
        return 1;
    }

    const double controlFactor = k * b0 * dt * dt;
    const double previousFactor = 2 * a2 + a1 * dt;
    double denominator = a2 + a1 * dt + a0 * dt * dt;
    // Без задержки управление содержит неизвестное x[i]: переносим его влево.
    if (delay == 0) denominator += controlFactor;
    trajectory << "t,x\n0,0\n";
    for (int i = 1; i <= steps; i++) {
        double previous2 = (i >= 2) ? x[i - 2] : 0;
        double measured = (delay > 0 && i >= delay) ? x[i - delay] : 0;
        x[i] = (previousFactor * x[i - 1] - a2 * previous2
                + controlFactor * (1 - measured)) / denominator;
        trajectory << i * dt << ',' << x[i] << '\n';
    }

    critical << "a1,tau_a2_0.5,tau_a2_1,tau_a2_1.5\n";
    for (int i = 5; i <= 30; i++) {
        double damping = i / 10.0;
        critical << damping;
        for (double inertia : {0.5, 1.0, 1.5})
            critical << ',' << criticalDelay(inertia, damping, a0, b0, k);
        critical << '\n';
    }
    trajectory.close();
    critical.close();
    if (!trajectory || !critical) {
        cerr << "Не удалось записать CSV-файлы полностью\n";
        return 1;
    }
    cout << "Готово: trajectory.csv — x(t), critical.csv — границы устойчивости\n";
}
