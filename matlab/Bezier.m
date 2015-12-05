function [ x, y ] = Bezier( tval, curve );
    x = ( 1 - tval )^3 * curve( 1, 1 ) + 3 * ( 1 - tval )^2 * tval * curve( 2, 1 ) + 3 * ( 1 - tval ) * tval^2 * curve( 3, 1 ) + tval^3 * curve( 4, 1 );
    y = ( 1 - tval )^3 * curve( 1, 2 ) + 3 * ( 1 - tval )^2 * tval * curve( 2, 2 ) + 3 * ( 1 - tval ) * tval^2 * curve( 3, 2 ) + tval^3 * curve( 4, 2 );
end