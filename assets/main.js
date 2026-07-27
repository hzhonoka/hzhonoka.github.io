/* ============ 灰空薄荷 · 公共脚本 ============ */

/* ---------- Giscus 评论 ----------
 * 在指定容器里加载一个 Giscus 评论区。term 用于区分不同页面的评论。 */
function loadGiscus(container, term) {
  const cfg = window.GISCUS_CONFIG || {};
  if (!cfg.repoId || cfg.repoId.indexOf("TODO") === 0) {
    container.innerHTML =
      '<p style="color:var(--ink-light);font-size:.8rem;font-family:var(--sans);padding:6px 0;">' +
      "评论区尚未配置，站长请按《部署教程》第 4 步接入 Giscus。</p>";
    return;
  }
  container.innerHTML = "";
  const s = document.createElement("script");
  s.src = "https://giscus.app/client.js";
  s.setAttribute("data-repo", cfg.repo);
  s.setAttribute("data-repo-id", cfg.repoId);
  s.setAttribute("data-category", cfg.category);
  s.setAttribute("data-category-id", cfg.categoryId);
  s.setAttribute("data-mapping", "specific");
  s.setAttribute("data-term", term);
  s.setAttribute("data-strict", "0");
  s.setAttribute("data-reactions-enabled", "1");
  s.setAttribute("data-emit-metadata", "0");
  s.setAttribute("data-input-position", "top");
  s.setAttribute("data-theme", "light");
  s.setAttribute("data-lang", "zh-CN");
  s.setAttribute("data-loading", "lazy");
  s.crossOrigin = "anonymous";
  s.async = true;
  container.appendChild(s);
}

/* ---------- 渲染顶部导航（全站统一） ---------- */
function renderHeader(activePage) {
  const cfg = window.SITE_CONFIG || {};
  const pages = [
    { href: "index.html", label: "碎碎念", key: "moments" },
    { href: "posts.html", label: "文章", key: "posts" },
    { href: "album.html", label: "相册", key: "album" },
    { href: "about.html", label: "关于", key: "about" },
  ];
  const nav = pages
    .map(
      (p) =>
        `<a href="${p.href}" class="${p.key === activePage ? "active" : ""}">${p.label}</a>`
    )
    .join("");
  document.getElementById("site-header").innerHTML = `
    <div class="header-inner">
      <a class="site-title" href="index.html">${cfg.siteName || "灰空薄荷"}<small>${cfg.tagline || ""}</small></a>
      <nav class="nav">${nav}</nav>
    </div>`;
}

/* ---------- 渲染页脚 ---------- */
function renderFooter() {
  const cfg = window.SITE_CONFIG || {};
  document.getElementById("site-footer").innerHTML = `
    ${cfg.siteName || "灰空薄荷"} · ${cfg.nickname || ""}<br>
    Powered by GitHub Pages`;
}
