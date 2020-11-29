% B = 41;
% peak_41 = 0.044
% steepness_new = 6
% B = 41 * 0.044 * 6;
hold off
hold on
for f = logspace(-0.6, 0.6, 5)
    B = 41 * f;
    C = 1.4;
    D = 1;
    E = -0.2;
    x = linspace(-1, 1, 100) * 20 / 180 * pi;
    plot(x * 180 / pi, D * sin(C * atan(B * x - E * (B * x - atan(B * x)))));
end
