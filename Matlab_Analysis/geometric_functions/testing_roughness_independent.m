%% Variables
studied_inc = 10:10:80;
studied_ref = (-pi/2 + 0.1):0.1:(pi/2-0.1);
roughness = 0.5:0.1:1.0;
N  = [0 1];

tt = tiledlayout(4,2);  % Defines 2by2 layout. 
tt.Padding = 'compact';
tt.TileSpacing = 'compact';
title(tt,'Roughness Independent Geometric Functions');
nexttile


%% Implicit Geometric Function
for incd = studied_inc*(pi/180)
    v = [cos(incd + pi/2) sin(incd + pi/2)];
    plotted_geom = zeros(size(studied_ref));
    for ri = 1:size(studied_ref,2)
        l = [cos(studied_ref(ri) + pi/2) sin(studied_ref(ri) + pi/2)];
        % l = -l;
        % l = l - 2.0 .* dot(N, l) .* N;
        H = v+l;
        H = H./norm(H);
        plotted_geom(ri) = implicit(l , v, H);
    end
    plot3(repelem(rad2deg(incd), size(studied_ref,2)), studied_ref.*180./pi, plotted_geom);
    title('implicit');
    xlabel('Incident Angle');
    ylabel('Reflection Angle');
    zlabel('Geometric Factor');
    hold on
end

hold off
nexttile

for incd = studied_inc*(pi/180)
    v = [cos(incd + pi/2) sin(incd + pi/2)];
    plotted_geom = zeros(size(studied_ref,2));
    for ri = 1:size(studied_ref,2)
        l = [cos(studied_ref(ri) + pi/2) sin(studied_ref(ri) + pi/2)];
        % l = -l;
        % l = l - 2.0 .* dot(N, l) .* N;
        H = v+l;
        H = H./norm(H);
        plotted_geom(ri) = implicit(l , v, H);
    end
    plot(studied_ref.*180./pi, plotted_geom);
    title('implicit');
    xlabel('Reflection Angle');
    ylabel('Geometric Factor');
    grid on
    hold on
end
hold off
nexttile





%% Neumann Geometric Function
for incd = studied_inc*(pi/180)
    v = [cos(incd + pi/2) sin(incd + pi/2)];
    plotted_geom = zeros(size(studied_ref));
    for ri = 1:size(studied_ref,2)
        l = [cos(studied_ref(ri) + pi/2) sin(studied_ref(ri) + pi/2)];
        % l = -l;
        % l = l - 2.0 .* dot(N, l) .* N;
        H = v+l;
        H = H./norm(H);
        plotted_geom(ri) = neumann(l , v, H);
    end
    plot3(repelem(rad2deg(incd), size(studied_ref,2)), studied_ref.*180./pi, plotted_geom);
    title('Neumann');
    xlabel('Incident Angle');
    ylabel('Reflection Angle');
    zlabel('Geometric Factor');
    hold on
end

hold off
nexttile

for incd = studied_inc*(pi/180)
    v = [cos(incd + pi/2) sin(incd + pi/2)];
    plotted_geom = zeros(size(studied_ref,2));
    for ri = 1:size(studied_ref,2)
        l = [cos(studied_ref(ri) + pi/2) sin(studied_ref(ri) + pi/2)];
        % l = -l;
        % l = l - 2.0 .* dot(N, l) .* N;
        H = v+l;
        H = H./norm(H);
        plotted_geom(ri) = neumann(l , v, H);
    end
    plot(studied_ref.*180./pi, plotted_geom);
    title('Neumann');
    xlabel('Reflection Angle');
    ylabel('Geometric Factor');
    grid on
    hold on
end
hold off
nexttile



%% Kelemenn Geometric Function
for incd = studied_inc*(pi/180)
    v = [cos(incd + pi/2) sin(incd + pi/2)];
    plotted_geom = zeros(size(studied_ref));
    for ri = 1:size(studied_ref,2)
        l = [cos(studied_ref(ri) + pi/2) sin(studied_ref(ri) + pi/2)];
        l = -l;
        l = l - 2.0 .* dot(N, l) .* N;
        plotted_geom(ri) = kelemen(l , v, N);
    end
    plot3(repelem(rad2deg(incd), size(studied_ref,2)), studied_ref.*180./pi, plotted_geom);
    title('Kelemen');
    xlabel('Incident Angle');
    ylabel('Reflection Angle');
    zlabel('Geometric Factor');
    hold on
end

hold off
nexttile

for incd = studied_inc*(pi/180)
    v = [cos(incd + pi/2) sin(incd + pi/2)];
    plotted_geom = zeros(size(studied_ref,2));
    for ri = 1:size(studied_ref,2)
        l = [cos(studied_ref(ri) + pi/2) sin(studied_ref(ri) + pi/2)];
        l = -l;
        l = l - 2.0 .* dot(N, l) .* N;
        plotted_geom(ri) = kelemen(l , v, N);
    end
    plot(studied_ref.*180./pi, plotted_geom);
    title('Kelemen');
    xlabel('Reflection Angle');
    ylabel('Geometric Factor');
    grid on
    hold on
end
hold off
nexttile



%% Cook-Torrance Geometric Function
for incd = studied_inc*(pi/180)
    v = [cos(incd + pi/2) sin(incd + pi/2)];
    plotted_geom = zeros(size(studied_ref));
    for ri = 1:size(studied_ref,2)
        l = [cos(studied_ref(ri) + pi/2) sin(studied_ref(ri) + pi/2)];
        l = -l;
        l = l - 2.0 .* dot(N, l) .* N;
        plotted_geom(ri) = cook_torrance(l , v, N);
    end
    plot3(repelem(rad2deg(incd), size(studied_ref,2)), studied_ref.*180./pi, plotted_geom);
    title('Cook-Torrance');
    xlabel('Incident Angle');
    ylabel('Reflection Angle');
    zlabel('Geometric Factor');
    hold on
end

hold off
nexttile

for incd = studied_inc*(pi/180)
    v = [cos(incd + pi/2) sin(incd + pi/2)];
    plotted_geom = zeros(size(studied_ref,2));
    for ri = 1:size(studied_ref,2)
        l = [cos(studied_ref(ri) + pi/2) sin(studied_ref(ri) + pi/2)];
        l = -l;
        l = l - 2.0 .* dot(N, l) .* N;
        plotted_geom(ri) = cook_torrance(l , v, N);
    end
    plot(studied_ref.*180./pi, plotted_geom);
    title('Cook-Torrance');
    xlabel('Reflection Angle');
    ylabel('Geometric Factor');
    grid on
    hold on
end
hold off
%nexttile









