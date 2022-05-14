

incidents = (-pi/2 + 0.1):0.1:(pi/2-0.1);
views = 0.0:deg2rad(20):(pi/2-0.1);
roughness = linspace(1, 400, 4);


t = tiledlayout(2,2); % Defines 2by2 layout. 
t.Padding = 'compact';
t.TileSpacing = 'compact';
title(t,'Diagnosis of Multiple Phong Model Arguments');



N = [0 1];

for rr = roughness
    r = round(rr);
    nexttile
    for vv = views
        V = [cos(vv + pi/2) sin(vv + pi/2)]; % View direction
        plotted_geom = zeros(size(incidents,2));
        for ii = 1:size(incidents,2)   
            L = [cos(incidents(ii) + pi/2) sin(incidents(ii) + pi/2)]; % incident
            R = 2.*dot(L,N)*N - L; % Reflection vector
            plotted_geom(ii) = max(0.0, dot(R,V))^r;
            
            %{
            p0 = [0 0]
            vectarrow(V, p0);
            hold on
            axis equal
            vectarrow(p0, N);
            hold on
            axis equal
            vectarrow(p0, R);
            hold on
            axis equal
            %}
        end
        plot3(repelem(rad2deg(vv), size(incidents, 2)), incidents.*180./pi, plotted_geom);
        temp = "N value of " + r + " ";
        title(temp);
        xlabel('View Angle');
        ylabel('Incident Direction');
        zlabel('Phong Specular Factor');
        hold on
    end
    hold off
end
    

