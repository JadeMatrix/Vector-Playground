function [ P1, P2, P3, P4 ] = BasicBezierField( vval, A_b, B_b, C_b, D_b );
    P1 = sym( 'P1', [ 1, 2 ] )
    P2 = sym( 'P2', [ 1, 2 ] )
    P3 = sym( 'P3', [ 1, 2 ] )
    P4 = sym( 'P4', [ 1, 2 ] )
    [ P1( 1 ), P1( 2 ) ] = Bezier( vval, D_b );
    [ P2( 1 ), P2( 2 ) ] = Lerp( vval, A_b( 2, 1:2 ), C_b( 3, 1:2 ) );
    [ P3( 1 ), P3( 2 ) ] = Lerp( vval, A_b( 3, 1:2 ), C_b( 2, 1:2 ) );
    [ P4( 1 ), P4( 2 ) ] = Bezier( vval, B_b );
end