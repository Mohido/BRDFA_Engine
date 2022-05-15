function [S] = cook_torrance(V, L, N, H, r, n)
    D = distribution_beckmann(H, N, r.*r);
    F = fresnel_cook_torrance(V, H, n);
    G = geometric_cook_torrance(V, L, N);
    S = D; %(D.*F.*G) ./ (4.*dot(L,N).*dot(V,N));
end

