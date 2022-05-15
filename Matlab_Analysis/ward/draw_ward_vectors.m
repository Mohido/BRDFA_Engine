function [] = draw_ward_vectors(V, N, i_ang_theta, i_ang_phi, alpha, beta, rho)

for theta = i_ang_theta
   for phi = i_ang_phi
       L = [sin(theta).*cos(phi), 
            sin(theta).*sin(phi),
            cos(theta)
            ];
        H = V + L;
        H = H./norm(H);
        
        D = ward_distribution_vector(V, L, N, H, alpha, beta, rho);
        
        vectarrow([0 0 0], L.*min(5,D));
        hold on
        %axis equal
   end
end
end

