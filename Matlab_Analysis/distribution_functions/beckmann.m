function [D] = beckmann(H, N, r)
%DISTRIBUTION_BECKMANN input: Middle vector, alpha -> Distribution of
%facets along vector H.
%   Beckmann distribution function returns the ratio of microfacets area
%   facing direction H. H is the normal of perfect reflection of a
%   microfacet. 
%   H = normalize(V+L). Where V is the view direction, and L is the light
%   sample direction (Incident direction).
a = r.*r;
exponentnom = (dot(H,N).^2 - 1);
exponentden = a*a*dot(H,N).^2;
nom = exp(exponentnom ./ exponentden);
den = 4.*a.*a.*dot(H,N).^4;

D = nom/den;
end

