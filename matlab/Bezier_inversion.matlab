syms t v x y
A = sym( 'A', [ 4, 2 ] )
B = sym( 'B', [ 4, 2 ] )
C = sym( 'C', [ 4, 2 ] )
D = sym( 'D', [ 4, 2 ] )
P = sym( 'P', [ 4, 2 ] )

[ P( 1, 1:2), P( 2, 1:2), P( 3, 1:2), P( 4, 1:2) ] = BezierField( v, A, B, C, D )

[ x, y ] = Bezier( t, P )

lookup = solve( [ x, y ], [ t, v ] )