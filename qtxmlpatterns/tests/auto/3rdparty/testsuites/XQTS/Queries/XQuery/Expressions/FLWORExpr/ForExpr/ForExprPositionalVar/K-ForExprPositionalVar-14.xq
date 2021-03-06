(:*******************************************************:)
(: Test: K-ForExprPositionalVar-14                       :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Verify that the position is properly computed for fn:insert-before()(#3). :)
(:*******************************************************:)
deep-equal((1, 2, 3, 4),
	    for $i at $p
	    in insert-before((1, current-time()), 2, (current-date(), 3))
	    return $p)