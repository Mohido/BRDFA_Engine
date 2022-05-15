function [F] = fresnel_cook_torrance(V, H, n)
    c = dot(V,H);
    g2 = n.*n + c.*c - 1;
    g = sqrt(g2);
    
    fstT = ((g-c)./(g+c)).^2;
    sndT = 1 + ((c.*(g+c)-1)./(c.*(g-c)+1)).^2;
    F = 0.5 .* fstT .* sndT;
end

