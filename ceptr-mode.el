;;; ceptr-mode.el --- sample major mode for editing ceptr semantic trees.

;; Copyright © 2016, by Eric Harris-Braun

;; Author: Eric Harris-Braun ( gbs@panix.com )
;; Version: 0.0.1
;; Created: 18 Apr 2016
;; Keywords: languages ceptr
;; Homepage: http://ceptr.org

;; This file is not part of GNU Emacs.

;;; License:

;; You can redistribute this program and/or modify it under the terms of the GNU General Public License version 2.

;;; Commentary:

;; major mode for editing text encoding of ceptr semantic trees

;;; Code:

(defvar ceptr-font-lock-defaults nil "Value for font-lock-defaults.")

(setq ceptr-font-lock-defaults
      '(("\(\\([a-z_A-Z0-9_]+\\)" . (1 font-lock-function-name-face))
;        ("\"\\([^ ]+?\\)\"" . (1 font-lock-string-face))
        (":\\([0-9.]+\\))" . (1 font-lock-keyword-face))         ;integer or float surface
        (":\\('.'\\))" . (1 font-lock-string-face))              ;char surface
        (":\\([a-z_A-Z0-9_]+\\)" . (1 font-lock-type-face))      ;symbol surface
        (":\\(/[0-9/]+\\)" . (1 font-lock-string-face))          ;path surface
        ("\\([a-z_A-Z0-9!@#$%^&*{}-]+\\)" . (1 font-lock-warning-face))                ;anything else
        ) )

(define-derived-mode ceptr-mode fundamental-mode "Ceptr"
  "Major  mode for editing Ceptr semantic trees"
  (setq-local font-lock-defaults '(ceptr-font-lock-defaults))
  (set (make-local-variable 'indent-line-function) 'lisp-indent-line)
  (set (make-local-variable 'indent-tabs-mode) nil)
  )
(progn
   (add-to-list 'auto-mode-alist '("\\.cptr\\'" . ceptr-mode))
)

(provide 'ceptr)
