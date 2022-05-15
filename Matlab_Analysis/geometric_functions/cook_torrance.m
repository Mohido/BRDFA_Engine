function [G] = cook_torrance(L, V, N)
    H = V+L;
    H = H./norm(H);
    masking = (2.* dot(N,H).* dot(N,V)) ./ dot(V,H);
    shadowing = (2.* dot(N,H).* dot(N,L)) ./ dot(V,H);
    G = min([1 masking shadowing]);
end

