(:*******************************************************:)
(: Test: K2-NodeTest-18                                  :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: schema-attribute() isn't allowed inside document-node(). :)
(:*******************************************************:)
document-node(schema-attribute(ncname))