function [G] = neumann(L,V,N)
    nom = dot(L,N) .* dot(N,V);
    den = max(dot(L,N), dot(N,V));
    G = nom ./ den;
    
end

