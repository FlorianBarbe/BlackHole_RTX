# BlackHole

## Pousser les modifications vers GitHub

1. Vérifie que ton dépôt local pointe bien vers le dépôt GitHub (par exemple `origin`).
2. Récupère les dernières mises à jour si nécessaire :
   ```bash
   git pull origin main
   ```
3. Ajoute les fichiers modifiés et crée un commit :
   ```bash
   git add .
   git commit -m "Ton message de commit"
   ```
4. Envoie le commit sur la branche GitHub :
   ```bash
   git push origin main
   ```
5. Si tu travailles via des branches ou des pull requests :
   - Crée une nouvelle branche locale avant de modifier le code : `git checkout -b ma-branche`.
   - Pousse-la vers GitHub : `git push -u origin ma-branche`.
   - Ouvre ensuite une pull request sur GitHub pour fusionner les changements.

Ces commandes permettent d’appliquer les modifications locales (y compris celles liées au modèle relativiste ajouté) sur ton dépôt GitHub.
