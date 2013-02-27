(ns vp.core)

(def instruction-set-words
  {:restart (fn [state _]
              (dosync
               (alter state assoc :pc 0)
               nil)
              )
   :pop (fn [state arguments]
          (dosync 
           (let [pc (:pc @state)
                 mem (:mem @state)]
             (if (>= pc (count mem)) 
               nil
               (let [word (nth mem pc)]
                 (alter state assoc :pc (+ pc 1))
                 word)))))})

(def stack-words 
  {:pop (fn [state _]
          (dosync (let [s (:stack @state)
                        val (last s)]
                    (alter state assoc :stack (pop s))
                    val)))
   :push (fn [state arg]
           (dosync (let [s (:stack @state)]
                     (alter state assoc :stack (conj s arg))
                     nil)))})

(def processor-words
  {:process (fn [state arg]
              (dosync (let [context (get @state :context)
                            process-flow (if (nil? arg) (get @state :process-flow) arg)
                            [instruction op1 op2] (tell-flow context process-flow :pop nil)
                            ]
                        (if (nil? instruction) (str "<Process Completed>")
                            (cond (= instruction :tell) 
                                  (let [[flow-id word args] op1]
                                    (tell-flow context flow-id word args)
                                    )
                                  (= instruction :link)
                                  (let [[src-flow-id src-word src-args] op1
                                        [dst-flow-id dst-word] op2
                                        dst-args (tell-flow context src-flow-id src-word src-args)]
                                    (tell-flow context dst-flow-id dst-word dst-args)
                                    )
                                  true (str "<Unknown instruction: " instruction ">"))
                            )
                        )))})

(def instructions
  [[:tell [:screen :push "hello flows"]]
   [:link [:keyboard :pop] [:stack :push]]
   [:link [:keyboard :pop] [:stack :push]]
   [:link [:stack :pop] [:screen :push]]
   [:link [:stack :pop] [:screen :push]]
   [:tell [:memory :restart]]])

(def the-context (ref {}))

(defn tell-flow
  "say something to a flow in it's protocol"
  [context flow-id word arguments]
  (let [flow (get @context flow-id)
        word-fn (word (:words flow))
        state (:state flow)]
    (println "telling" flow-id "to" word arguments)
    (word-fn state arguments)
    ))

(defmacro register-flows [context & list] `(dosync (alter ~context assoc ~@list)))

(register-flows the-context
                :screen {:words {:push (fn [_ arguments] (println arguments) nil)}}
                :keyboard {:words {:pop (fn [_ _] (read-line))}}
                :stack {:words stack-words
                        :state (ref {:stack []})}
                :memory {:words instruction-set-words
                         :state (ref {:pc 0
                                      :mem instructions})}
                :_ {:words {:pop (fn [_ _] [:tell [:processor :process]])}}
                :processor {:words processor-words
                            :state (ref {:process-flow :memory
                                         :context the-context})})


(defn step
  "Do one process step"
  []
  (tell-flow the-context :processor :process :_))

