(:*******************************************************:)
(:Test: compare2args-4                                    :)
(:Written By: Carmelo Montanez                            :)
(:Date: Fri Dec 10 10:15:46 GMT-05:00 2004                :)
(:Purpose: Evaluates The "compare" function              :)
(: with the arguments set as follows:                    :)
(:$comparand1 = xs:string(lower bound)                   :)
(:$comparand2 = xs:string(mid range)                     :)
(:*******************************************************:)

fn:compare(xs:string("This is a characte"),xs:string("This is a characte"))