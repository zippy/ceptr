(defn last-str [data]
 )
(defn respond [str])
(defn get-match-ptr [str])
(defn pass)

(defstate nil [data]
  (defcond [data]
    {:cond (re-match (last-str data) #"^HELO")
     :method (respond "250 OK")
     :logging (state :open)}
    )
  (defcond [data]
    {:cond (re-match (last-str data) #"^EHLO")
     :method (respond "250 OK\nEHLO\nHELO\n..")
     :logging (state :open)}
    )
  (defcond [data]
    {:cond nil
     :method (respond "500 Syntax Error")
     :logging nil}
    )
  )

(def rset-cond (defcond [data status]
                 {:cond (re-match (last-str data) #"RSET")
                  :method (respond "250 OK")
                  :logging (state :open)}
                 ))

(defstate :open [data]
  rset-cond
  (defcond [data]
    {:cond (re-match (last-str data) #"^MAIL( FROM)*:*")
     :method (pass :email-parser
                   {:id (get-id data)
                    :data-ptr (get-match-ptr (last-str data) #"^MAIL( FROM)*:*")
                    :result-state {:ok (defcond {:method (respond "250 OK")
                                                  :logging (state :from)})
                                    :_else (defcond {:method (respond "501 Syntax Error")
                                                     :logging (state :open)})
                                    }})
    :logging (state :passed "Waiting for FROM parse")}
    ))

(def recpt-cond
  rset-cond
  (defcond [data status]
    {:cond (re-match (last-str data) #"^RCPT( TO)*:*")
     :method (pass :email-parser
                   {:id (get-id data)
                    :data-ptr (get-match-ptr (last-str data) #"RCPT( TO)*:*")
                    :result-state {:ok (defcond {:method (respond "250 OK")
                                                 :logging (state :rcpt)})
                                   :_else (defcond {:method (respond "501 Syntax Error")
                                                    :logging (state :rcpt)})
                                    }})
     :logging (state :passed "Waiting for RCPT parse")}
    ))

(defstate :from
  rcpt-cond
  rset-cond
)

(defstate :rcpt
  rcpt-cond
  rset-cond
  (defcond [data]
    {:cond (re-match (last-str data) #"^DATA:*")
     :method (respond "354 Start mail input; end with <CRLF>.<CRLF>")
     :logging (state :data "getting data")}
    ))


(defstate :data
  rset-cond
  (defcond [data]
    {:cond (re-match (last-str data) #"<CRLF>.<CRLF>")
     :method (parse)
     :logging (state :recognized)}
    )
  (defcond [data]
    {:cond nil
     :method (respond "250 OK")
     :logging nil}
    ))
