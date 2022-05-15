function [G] = kelemen(L, V, N)
    H = V+L;
    H = H./norm(H);
    nom = dot(L,N) .* dot(N,V);
    den = dot(V,H).^2;
    G = nom./den;
end

