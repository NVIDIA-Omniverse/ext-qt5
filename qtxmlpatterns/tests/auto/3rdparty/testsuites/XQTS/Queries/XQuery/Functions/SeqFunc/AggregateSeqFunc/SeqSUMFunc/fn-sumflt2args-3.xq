(:*******************************************************:)
(:Test: sumflt2args-3                                     :)
(:Written By: Carmelo Montanez                            :)
(:Date: Fri Dec 10 10:15:47 GMT-05:00 2004                :)
(:Purpose: Evaluates The "sum" function                  :)
(: with the arguments set as follows:                    :)
(:$arg = xs:float(lower bound)                           :)
(:$zero = xs:float(mid range)                            :)
(:*******************************************************:)

fn:sum((xs:float("-3.4028235E38"),xs:float("0")))