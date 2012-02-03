ACSRF: Advanced Cross Site Request Forgery
==========================================

Introduction
============

Les <em>Advanced CSRF</em> sont n&eacute;es suite &agrave; quelques recherches sur les <em>Cross Site Request Forgery</em> effectuees au sein du lab.</p>

Les <em>ACSRF</em> sont une utilisation avanc&eacute;e des <em>CSRF</em>; En ajoutant la possibilit&eacute; de pouvoir r&eacute;cuperer le <em>temps de calcul</em> de la page redirig&eacute;e, il est alors possible de r&eacute;cuperer des informations provenants d'une base de donn&eacute;e sur laquelle est connect&eacute;e une page web contenant une injection SQL.

Les particularit&eacute;s de ce type d'attaque sont:

* C'est la victime qui apparait dans les fichiers logs du site contenant l'injection SQL
* D&eacute;centralisation d'une attaque
* Difficilement/pas patchable
* Forensics difficile
* Nouveau type d'attaque desormais possible: Sql injections parties administration

Papiers
=======

* <a href="2008-Epitech-SecurityLab_AdvancedCSRF.pdf">2008-Epitech-SecurityLab_AdvancedCSRF.pdf</a>:  Papier technique sur les ACSRF.
* <a href="SSTIC08_Guasconi-Touron_AdvancedCSRF_slides.pdf">SSTIC08_Guasconi-Touron_AdvancedCSRF_slides.pdf</a>: Rump session present&eacute;e lors du SSTIC 2008.

Preuves de concept
==================

* <a href="acsrf-server_1.0.tar.bz2">acsrf-server_1.0.tar.bz2</a>

Serveur multiplex&eacute; capable de faire des <em>Advanced CSRF</em> par sessions.

Le serveur bloque toutes les sockets de l'application sauf une. Il se sert alors de la socket disponible pour redirig&eacute;e la cible sur l'application &agrave; attaquer.

Il compare ensuite le temps que met la cible pour recontacter le serveur (temps de calcul de la page), il est donc permis de faire des Attaques de type Blind SQL injections.

Grace &agrave; un syst&egrave;me de sessions, les attaques peuvent &ecirc;tre cibl&eacute;es (par ips), de programmer plusieurs attaques, de faire des CSRF classiques (sans notion de temps de calcul) ou tout simplement de faire une redirection sur une image existante, ce qui permet de camoufler l'attaque sur une page au rechargement de la page, une fois l'attaque termin&eacute;e.

* <a href="server-demo-fr-www.html">Example illustr&eacute;</a> d'attaque via une page html contenant des images.
* <a href="server-demo-fr-mail.html">Screenshots</a> d'une attaque par email sous thunderbird.
* <a href="server-demo-fr-gmail.html">Screenshots</a> d'une attaque par email sous gmail.

Il est potentiellement possible de reproduire ce type d'attaques sur d'autres clients/protocoles; Il suffit simplement que ces derniers possedent un nombre de threads limit&eacute; et qu'ils supportent le chargement d'image externes et donc les redirections HTTP. <em>(nntp, .pdf, .doc, .xls)</em>

<a href="inject_html.js">inject_html.js</a>
Proof of concept des blind sql injections basees sur le temps de reponse d'une requete redirigee.

Ce script se contente de creer un objet image javascript qu'il redirige vers la page a tester, en ajoutant un chronometre avant et apres la redirection vers le site victime.

Dans cette example encore, c'est le visiteur de la page web qui attaque victim.com et donc apparait dans les logs a notre place.

Infos
=====

Auteur: <a href="http://esl.epitech.net/~moul">Manfred Touron</a>

Avec l'aide de: <a href="http://esl.epitech.net/~tyop">Vincent Guasconi</a> pour la documentation

<a href="http://esl.epitech.net/">Epitech Security Lab.</a>
