(:*******************************************************:)
(:Test: minint2args-5                                     :)
(:Written By: Carmelo Montanez                            :)
(:Date: Fri Dec 10 10:15:47 GMT-05:00 2004                :)
(:Purpose: Evaluates The "min" function                  :)
(: with the arguments set as follows:                    :)
(:$arg1 = xs:int(lower bound)                            :)
(:$arg2 = xs:int(upper bound)                            :)
(:*******************************************************:)

fn:min((xs:int("-2147483648"),xs:int("2147483647")))