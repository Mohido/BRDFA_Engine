%% Plotting the lambartian model
roughness = 0;

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
