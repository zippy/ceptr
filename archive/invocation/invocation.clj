(defn transform [txt]
  (map #(shuffle %) (shuffle (map #(clojure.string/split % #"[ ,;]") (clojure.string/split txt #"[.?]" )))) )
(defn express [words]
  (println words))
(defn request [txt]
  (let [words (transform txt)]
    (express words)))

# (transform "Let me speak with you about things unspoken.  Let me touch that dance undanced.  Lets play the play we have yet to play, and I promise the softness of my hands, and real things really made.e")
# (["you" "about" "me" "Let" "things" "with" "speak" "unspoken"] ["hands" "softness" "yet" "I" "and" "" "" "the" "play" "we" "play" "real" "" "really" "have" "my" "to" "and" "things" "made" "of" "promise" "play" "Lets" "" "the"] ["me" "" "Let" "undanced" "dance" "" "touch" "that"])
