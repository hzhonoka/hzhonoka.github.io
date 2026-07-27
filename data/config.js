/* ============ 站点配置：改成你自己的信息 ============ */
window.SITE_CONFIG = {
  // 站名与签名
  siteName: "灰空薄荷",
  tagline: "灰色的天空下，薄荷轻轻摇曳。",

  // 你的昵称（显示在每条碎碎念前面）
  nickname: "hzhonoka",
  // 头像路径（把你的头像图片放到 assets/ 下，改成对应文件名即可）
  avatar: "assets/avatar.svg",

  // ============ 主页（门面页）文字 ============
  // 主页背景图：assets/bg.svg，想换成自己的图片就把同名文件覆盖，
  // 或改 index.html 里 .landing-hero 的 background-image 路径
  greeting: "你好，欢迎来到",   // 站名上方的小字
  welcome: "这里是 hzhonoka 的小站。写点文字，放点照片，记录一些不想忘掉的日子。\n随便逛逛，坐一会儿再走。",  // 欢迎语，\n 表示换行
};

/* ============ Giscus 评论配置 ============
 * 部署后按《部署教程.md》第 4 步操作，把下面 4 项换成 giscus.app 给你的值。
 * 没填好之前评论区域会显示提示，不影响其他功能。 */
window.GISCUS_CONFIG = {
  repo: "hzhonoka/hzhonoka.github.io", // 你的仓库名
  repoId: "TODO_REPO_ID",              // ← 替换
  category: "General",
  categoryId: "TODO_CATEGORY_ID",      // ← 替换
};
