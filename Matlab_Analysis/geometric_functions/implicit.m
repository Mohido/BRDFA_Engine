function [G] = implicit(L, V, N)
    G = dot(L,N) .* dot (N, V);
end

