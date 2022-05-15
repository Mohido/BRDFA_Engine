function [G] = smith_schlick_ggx(L, V, N, r)
    function [G] = schlick_ggx(V,N,r)
       a  = r.* r;
       k = a./2;
       
       VN = dot(V,N);
       den = VN .* (1-k) + k;
       G = VN ./ den;        
    end
    G = schlick_ggx(V,N,r) .* schlick_ggx(L,N,r);
end

