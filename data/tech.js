/* ============ 技术板块文章列表 ============
 * 写新技术笔记：在 tech-posts/ 文件夹里新建一个 .md 文件（用 Markdown 写），
 * 然后在下面最上方加一条记录：slug 填文件名（不带 .md）。
 * 注意逗号别漏掉！ */
window.TECH_POSTS = [
  //demystifying_agent_skills_notes
  {
    slug: "open_addressing_notes",
    title: "Optimal Bounds for Open Addressing Without Reordering",
    date: "2026-08-21",
    excerpt: "在不重新排序元素的前提下，开放寻址哈希表仍可实现远优于传统认知的探测复杂度。",
    tags: ["算法", "论文笔记"],
  },
  {
    slug: "demystifying_agent_skills_notes",
    title: "Demystifying Agent Skills: Why They Work—Until They Don’t",
    date: "2026-08-21",
    excerpt: "发现 Skill 主要不是注入新知识，而是把嘈杂经验锚定为稳定执行流程；同时揭示了检索精度随库规模暴跌、以及 Skill 在跨框架迁移中的独特优势。",
    tags: ["agent", "论文笔记"],
  },
  {
    slug: "LittleLearner： Language Models Under Pedagogically Controlled Knowledge Exposure",
    title: "LittleLearner： Language Models Under Pedagogically Controlled Knowledge Exposure",
    date: "2026-08-20",
    excerpt: "故意把大语言模型\"养笨\"，只让它学小学五年级及以下的东西，然后观察它到底能不能通过后训练、上下文学习等方式\"变聪明\"。",
    tags: ["大模型", "论文笔记"],
  },
  {
    slug: "cpp-notes-1",
    title: "C++ 学习笔记（一）：示例笔记",
    date: "2026-07-27",
    excerpt: "一篇放在「技术」板块的示例笔记，换成你自己的学习内容吧。",
    tags: ["C++", "笔记"],
  },
];
