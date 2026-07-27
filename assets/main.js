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

/* ---------- 页面淡入淡出过渡 ---------- */
(function () {
  document.documentElement.classList.add("page-enter");
  window.addEventListener("DOMContentLoaded", function () {
    requestAnimationFrame(function () {
      document.documentElement.classList.add("page-enter-active");
    });
  });
  // 站内 .html 链接点击时先淡出再跳转
  document.addEventListener("click", function (e) {
    const a = e.target.closest("a[href]");
    if (!a) return;
    const href = a.getAttribute("href");
    if (!href || a.target === "_blank" || href.startsWith("http") || href.startsWith("#") || href.startsWith("mailto:")) return;
    if (!/\.html(\?|#|$)/.test(href)) return;
    e.preventDefault();
    document.documentElement.classList.add("page-leave");
    setTimeout(function () { location.href = href; }, 220);
  });
})();

/* ---------- 渲染顶部导航（全站统一） ---------- */
function renderHeader(activePage) {
  const cfg = window.SITE_CONFIG || {};
  const pages = [
    { href: "moments.html", label: "碎碎念", key: "moments" },
    { href: "posts.html", label: "文章", key: "posts" },
    { href: "tech.html", label: "技术", key: "tech" },
    { href: "study.html", label: "学习", key: "study" },
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

/* ---------- 文章列表渲染 ----------
 * 三个列表页（文章/技术/学习）共用这个函数。
 * category 传 "essay" / "tech" / "study" 过滤，传 null 显示全部。 */
const CATEGORY_LABELS = { essay: "随笔", tech: "技术", study: "学习" };

function renderPostCards(listEl, posts, category) {
  const filtered = category
    ? posts.filter((p) => (p.category || "essay") === category)
    : posts;
  if (filtered.length === 0) {
    listEl.innerHTML =
      '<div class="card"><p style="color:var(--ink-light);font-size:.9rem;">' +
      "这里还没有文章，第一篇正在路上。</p></div>";
    return;
  }
  filtered.forEach(function (p) {
    const a = document.createElement("a");
    a.className = "card post-item";
    a.href = "post.html?p=" + encodeURIComponent(p.slug);
    const catLabel = CATEGORY_LABELS[p.category || "essay"] || "随笔";
    const tags = (p.tags || []).map((t) => `<span class="tag">${t}</span>`).join("");
    a.innerHTML =
      `<h2>${p.title}</h2>` +
      `<div class="post-date">${p.date} · ${catLabel}</div>` +
      `<div class="post-excerpt">${p.excerpt || ""}</div>` +
      (tags ? `<div class="post-tags">${tags}</div>` : "");
    listEl.appendChild(a);
  });
}

/* ---------- 渲染页脚 ---------- */
function renderFooter() {
  const cfg = window.SITE_CONFIG || {};
  document.getElementById("site-footer").innerHTML = `
    ${cfg.siteName || "灰空薄荷"} · ${cfg.nickname || ""}<br>
    Powered by GitHub Pages`;
}
