(:**************************************************************:)
(: Test: functx-fn-seconds-from-duration-2                                  :)
(: Written by: Priscilla Walmsley (Frans Englich is maintainer) :)
(: Date: 2008-05-16+02:00                                       :)
(:**************************************************************:)

declare namespace functx = "http://www.example.com/";
(seconds-from-duration(
   xs:dayTimeDuration('-PT90.5S')))
