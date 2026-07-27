/* ============ 文章列表 ============
 * 写新文章：在 posts/ 文件夹里新建一个 .md 文件（用 Markdown 写），
 * 然后在下面最上方加一条记录：slug 填文件名（不带 .md）。
 *
 * category 决定文章出现在哪个板块：
 *   "essay" → 随笔（只在「文章」页显示）
 *   "tech"  → 同时出现在「技术」板块
 *   "study" → 同时出现在「学习」板块
 * 「文章」页永远显示全部。
 * 注意逗号别漏掉！ */
window.POSTS = [
  {
    slug: "hello-world",
    title: "你好，灰空薄荷",
    date: "2026-07-27",
    category: "essay",
    excerpt: "小站的第一篇文章：这里都有什么，以及我是怎么把它搭起来的。",
    tags: ["博客", "建站"],
  },
  {
    slug: "cpp-notes-1",
    title: "C++ 学习笔记（一）：示例笔记",
    date: "2026-07-27",
    category: "tech",
    excerpt: "一篇放在「技术」板块的示例笔记，换成你自己的学习内容吧。",
    tags: ["C++", "笔记"],
  },
  {
    slug: "markdown-guide",
    title: "写文章小抄：Markdown 常用语法",
    date: "2026-07-27",
    category: "study",
    excerpt: "给自己看的备忘录：标题、加粗、代码块、图片、引用都怎么写。",
    tags: ["备忘录"],
  },
];
