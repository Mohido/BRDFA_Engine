%% Variables
studied_inc = 10:10:80;
studied_ref = (-pi/2 + 0.1):0.1:(pi/2-0.1);
roughness = linspace(0.1, 1.0 , 3);
N  = [0 1];

tt = tiledlayout(3, 2);  % Defines 2by2 layout. 
tt.Padding = 'compact';
tt.TileSpacing = 'compact';
title(tt,'Smith-GGX Geometric Function');


%% Beckmann Geometric Function
for r = roughness 
    nexttile
    for incd = studied_inc*(pi/180)
        v = [cos(incd + pi/2) sin(incd + pi/2)];
        plotted_geom = zeros(size(studied_ref));
        for ri = 1:size(studied_ref,2)
            l = [cos(studied_ref(ri) + pi/2) sin(studied_ref(ri) + pi/2)];
            % l = -l;
            % l = l - 2.0 .* dot(N, l) .* N;
            H = v+l;
            H = H./norm(H);
            plotted_geom(ri) = smith_ggx(l , v, H, r);
        end
        plot3(repelem(rad2deg(incd), size(studied_ref,2)), studied_ref.*180./pi, plotted_geom);
        temp = ("Roughness " + r);
        title(temp);
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
            plotted_geom(ri) = smith_ggx(l , v, H, r);
        end
        plot(studied_ref.*180./pi, plotted_geom);
        temp = ("Roughness " + r);
        title(temp);
        xlabel('Reflection Angle');
        ylabel('Geometric Factor');
        grid on
        hold on
    end
    hold off
end
