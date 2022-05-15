function [] = draw_cook_vectors(V, N, i_ang_theta, i_ang_phi, r, n)
hold off
nexttile
draw_axis();
axis equal
vectarrow(V.*2, [0 0 0]);
hold on
axis equal
for theta = i_ang_theta
   for phi = i_ang_phi
       L = [sin(theta).*cos(phi) sin(theta).*sin(phi) cos(theta)];
        L = -L;
        L = L - 2.0 .* dot(N, L) * N;
        H = V + L;
        H = H./norm(H);
        D = cook_torrance(V, L, N, H, r, n);
        vectarrow([0 0 0], L.*min(5,D));
        title("Cook-Torrance Specular BRDF");
        hold on
        axis equal
   end
end


% Drawing the Distribution Function
nexttile
for theta = i_ang_theta
   for phi = i_ang_phi
        L = [sin(theta).*cos(phi) sin(theta).*sin(phi) cos(theta)];
        L = -L;
        L = L - 2.0 .* dot(N, L) * N;
        H = V + L;
        H = H./norm(H);
        D = distribution_beckmann(H, N, r.*r);
        vectarrow([0 0 0], L.*min(5,D));
        title("Distribution Function");
        hold on
        axis equal
   end
end


% Drawing the Fresnel Function
nexttile
for theta = i_ang_theta
   for phi = i_ang_phi
        L = [sin(theta).*cos(phi) sin(theta).*sin(phi) cos(theta)];
        L = -L;
        L = L - 2.0 .* dot(N, L) * N;
        H = V + L;
        H = H./norm(H);
        D = fresnel_cook_torrance(V, H, n);
        vectarrow([0 0 0], L.*min(5,D));
        title("Fresnel Function");
        hold on
        axis equal
   end
end

% Drawing the Geometric Function
nexttile
for theta = i_ang_theta
   for phi = i_ang_phi
        L = [sin(theta).*cos(phi) sin(theta).*sin(phi) cos(theta)];
        L = -L;
        L = L - 2.0 .* dot(N, L) * N;
        H = V + L;
        H = H./norm(H);
        D = geometric_cook_torrance(V, L, N);
        vectarrow([0 0 0], L.*min(5,D));
        title("Geometric Function");
        hold on
        axis equal
   end
end

end