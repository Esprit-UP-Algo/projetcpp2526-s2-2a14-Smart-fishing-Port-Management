-- Exécutez ce script dans Oracle SQL Developer pour ajouter les nouvelles colonnes

ALTER TABLE EMPLOYEES
ADD (
    TELEPHONE VARCHAR2(8),
    EMAIL     VARCHAR2(100)
);

ALTER TABLE EMPLOYEE
ADD (
    TELEPHONE VARCHAR2(8),
    EMAIL     VARCHAR2(100)
);

ALTER TABLE FRIGOS
ADD (
    TELEPHONE VARCHAR2(20)
);

-- Ajout du lien vers le pêcheur (Employé)
ALTER TABLE PECHES
ADD (
    ID_PECHEUR NUMBER
);

-- Nouvelles colonnes pour le module Livraison
ALTER TABLE LIVRAISONS
ADD (
    REFERENCE    VARCHAR2(50)
);

