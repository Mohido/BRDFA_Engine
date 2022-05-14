function [G] = smith_beckmann(L, V, N, r)
    function [G_] = g_beckmann(V,N,r)
         a = r.*r;
        nem = dot(N,V);
        den = a.* sqrt((1 - dot(N,V).^2));
        c = nem./den;
        
        if(c <= 1.6)
            G_ = 3.535.*c + 2.181.*c.*c;
            G_ = G_ ./ (1 + 2.276.*c + 2.577.*c.*c);
        else
            G_ = 1;
        end
    end
   G = g_beckmann(V,N,r) .* g_beckmann(L,N,r);
end

