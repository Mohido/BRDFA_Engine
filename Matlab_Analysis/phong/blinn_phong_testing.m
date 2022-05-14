%% 1 Graph for different n argument

incidents = (-pi/2 + 0.1):0.1:(pi/2-0.1);
views = 0.0:deg2rad(20):(pi/2-0.1);
roughness = linspace(1, 100, 20);
N = [0 1];


tt = tiledlayout(1,2); % Defines 2by2 layout. 
tt.Padding = 'compact';
tt.TileSpacing = 'compact';
title(tt,'Diagnosis of Multiple Blinn-Phong Model Arguments');
nexttile


for rr = roughness
    r = round(rr);
    
    V = [cos(pi/2) sin(pi/2)]; % View direction
    plotted_geom = zeros(size(incidents,2));
    for ii = 1:size(incidents,2)   
        L = [cos(incidents(ii) + pi/2) sin(incidents(ii) + pi/2)]; % incident
        H = L+V;
        H = H./norm(H);
        
        plotted_geom(ii) = max(0.0, dot(H,N))^r;

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
    plot3(repelem(r, size(incidents, 2)), incidents.*180./pi, plotted_geom);
    xlabel('N value');
    ylabel('Incident Direction');
    zlabel('Blinn-Phong Specular Factor');
    hold on  
end

hold off
nexttile

for rr = roughness
    r = round(rr);
    V = [cos(pi/2) sin(pi/2)]; % View direction
    plotted_geom = zeros(size(incidents,2));
    for ii = 1:size(incidents,2)   
        L = [cos(incidents(ii) + pi/2) sin(incidents(ii) + pi/2)]; % incident
        H = L+V;
        H = H./norm(H);
        
        plotted_geom(ii) = max(0.0, dot(H,N))^r;

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
    plot(incidents.*180./pi, plotted_geom);
    xlabel('Incident Direction');
    ylabel('Blinn-Phong Specular Factor');
    hold on  
end
hold off
