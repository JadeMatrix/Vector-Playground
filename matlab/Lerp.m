function [ x, y ] = Lerp( lval, from_p, to_p );
    x = lval * from_p( 1 ) + ( 1 - lval ) * to_p( 1 );
    y = lval * from_p( 2 ) + ( 1 - lval ) * to_p( 2 );
end