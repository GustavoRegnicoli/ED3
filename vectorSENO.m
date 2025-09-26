N = 1000;              % cantidad de puntos
A = 1024;               % amplitud máxima del DAC de 12 bits
theta = linspace(0, 2*pi, N);
seno = round((A/2) * (sin(theta) + 1));  % +1 para que esté siempre positivo
fprintf('uint32_t Seno[%d] = {', length(seno));
fprintf('%d,', seno(1:end-1));   % todos menos el último con coma
fprintf('%d};\n', seno(end));    % último sin coma