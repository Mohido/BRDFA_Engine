%% Plotting Beckmann distribution function for multiple roughness values.
middle_angles = (-pi/2 + 0.1):0.1:(pi/2-0.1); % (-pi/2 + 0.1):0.1:(pi/2-0.1);
roughness = 0.5:0.1:1.0;
for r = roughness
    alpha = r*r;
    plotted_geom = zeros(size(middle_angles));
    
    for mai = 1:size(middle_angles,2)   
        h = [cos(middle_angles(mai) + pi/2) sin(middle_angles(mai) + pi/2)];
        plotted_geom(mai) = distribution_beckmann(h, [0 1] , alpha);
        %{
        p0 = [0 0];
        vectarrow(v, p0);
        hold on
        axis equal
        vectarrow(p0, [0 1]);
        hold on
        axis equal
        vectarrow(p0, h);
        hold on
        axis equal
        vectarrow(p0, l);
        hold on
        axis equal
         %}
    end

    % hold off

    % disp(plotted_geom)
    plot3(repelem(r, size(middle_angles, 2)), middle_angles.*180./pi, plotted_geom);
    title('Beckmann Distribution Function for Multiple Half Vector Angles');
    xlabel('Roughness Value');
    ylabel('Half Vector Angle (H)');
    zlabel('Distribution Coeffecient');
    hold on
end
hold off

%% Plotting The geometric attenuation function with multiple half vectors
studied_inc = 10:10:80;
studied_ref = (-pi/2 + 0.1):0.1:(pi/2-0.1);
for incd = studied_inc*(pi/180)
    v = [cos(incd + pi/2) sin(incd + pi/2)];
    plotted_geom = zeros(size(studied_ref));
    for ri = 1:size(studied_ref,2)
        l = [cos(studied_ref(ri) + pi/2) sin(studied_ref(ri) + pi/2)];
        plotted_geom(ri) = geometric_cook_torrance(v,l,[0 1]);
    end
    plot3(repelem(rad2deg(incd), size(studied_ref,2)), studied_ref.*180./pi, plotted_geom);
    title('Cook-Torrance Geometric Attenuations for Multiple Incident Angles');
    xlabel('Incident Angle');
    ylabel('Reflection Angle');
    zlabel('Geometric Factor');
    hold on
end
hold off

%% Plotting the Fresnel Approximation
studied_inc = 10:10:90;
studied_ref = (-pi/2 + 0.1):0.1:(pi/2-0.1);
refraction_indices = linspace(0.05, 0.95 , 4);
N  = [0 1];

tt = tiledlayout(4, 2);  % Defines 2by2 layout. 
tt.Padding = 'compact';
tt.TileSpacing = 'compact';

title(tt,'Schlick VS Cook-Torrance Fresnel Functions');

for F0=refraction_indices
    nexttile;
    %F0 = 0.9; %((refract - 1)./(refract + 1)).^2
    F_sqrt = sqrt(F0);
    n = (1+F_sqrt)./(1-F_sqrt);
    plotted_geom = zeros(size(studied_inc,2));
    plotted_geom_cook = zeros(size(studied_inc,2));
    ri = 1;
    for incd = studied_inc*(pi/180)
        v = [cos(incd + pi/2) sin(incd + pi/2)];
        %plotted_geom = zeros(size(studied_ref));
        
        plotted_geom(ri) = schlick_fresnel(v, N, F0);
        plotted_geom_cook(ri) = fresnel_cook_torrance(v, N, n);
        %{
        for ri = 1:size(studied_ref,2)
            l = [cos(studied_ref(ri) + pi/2) sin(studied_ref(ri) + pi/2)];
            
            H = v+l;
            H = H./norm(H);

            % Refraction to Base refraction
            
            plotted_geom(ri) = schlick_fresnel(l, H, F0);
        end
        % plot3(repelem(rad2deg(incd), size(studied_ref,2)), studied_ref.*180./pi, plotted_geom);
        
        temp = "IOR " + refract;
        title(temp);
        %zlim([0 1]);
        ylim([0 1]);
        xlabel('Incident Angle');
        %ylabel('Reflection Angle');
        ylabel('Reflection Intensity');
        %zlabel('Fresnel Factor');
        grid on
        hold on
        %}
        ri = ri+1;
    end
    plot(studied_inc, plotted_geom);
    temp = "IOR " + n + ", F0 " + F0;
    title(temp);
    ylim([0 1]);
    yticks(0:0.1:1);
    xlabel('Incident Angle');
    ylabel('Reflection Intensity');
    grid on
    hold off
    
    nexttile;
    plot(studied_inc, plotted_geom_cook);
    temp = "IOR " + n + ", F0 " + F0;
    title(temp);
    ylim([0 1]);
    yticks(0:0.1:1);
    xlabel('Incident Angle');
    ylabel('Reflection Intensity');
    grid on
    hold off
end

%% Plotting Cook-Torrance Vector form for multiple 